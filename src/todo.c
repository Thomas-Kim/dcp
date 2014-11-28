#include "todo.h"

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <sys/stat.h>
#include <unistd.h>

const size_t buf_size = 0x400; // Buffer size for paths

typedef struct todo {
    struct stat info;
    char* path;
    #ifdef __cplusplus
    // how to order items
    bool operator < (const todo& other) const {
        return info.st_size > other.info.st_size;
    };
    #endif // __cplusplus
} todo_t;
typedef todo_t* todo_t_ptr;

// TODO restructure so that end is a pointer to and invalid node and not a pointer to a pointer
#define mpsc(type)\
struct mpsc##type {\
    struct node##type  **begin, **end, *start;\
};\
struct node##type {\
    struct node##type* next;\
    size_t count; \
    type data;\
};\
\
size_t count_from##type(struct node##type** target) {\
    struct node##type* node = (struct node##type*) target;\
    size_t prev_count = node->count;\
    return prev_count;\
}\
static void update_tail##type(struct mpsc##type* mpsc, size_t count, struct node##type** proposed) {\
    size_t proposed_count = count_from##type(proposed);\
    while (1) {\
        struct node##type** current_tail = mpsc->end;\
        size_t current_count = count_from##type(current_tail);\
        if (current_count > proposed_count) {\
            return;\
        }\
        if (atomic_compare_exchange_strong(&mpsc->end, &current_tail, proposed)) {\
            return;\
        }\
    }\
}\
\
static void put##type(struct mpsc##type* mpsc, type todo) {\
    struct node##type* put = malloc(sizeof(struct node##type));\
    put->data = todo;\
    put->next = NULL;\
    struct node##type** target;\
    struct node##type* expected;\
    try_node:\
    target = mpsc->end;\
    expected = NULL;\
    put->count = count_from##type(target);\
    if (!atomic_compare_exchange_strong(target, &expected, put)) {\
        goto try_node;\
    }\
    update_tail##type(mpsc, put->count, &put->next);\
}\
static void clean_one##type(struct mpsc##type* mpsc) {\
    struct node##type* to_free = mpsc->start;\
    mpsc->start = to_free->next;\
    free(to_free);\
}\
\
static type get##type(struct mpsc##type* mpsc) {\
    if (*mpsc->begin) {\
        struct node##type* got = *mpsc->begin;\
        type ret = got->data;\
        mpsc->begin = &got->next;\
        clean_one##type(mpsc);\
        return ret;\
    }\
    return NULL;\
}\
struct mpsc##type
mpsc(todo_t_ptr) dirs, files;
volatile size_t available, in_progress;
void todo_init() {
    // TODO a constructor-like thing?
    dirs.start = malloc(sizeof(*dirs.start));
    dirs.end = &dirs.start->next;
    dirs.start->next = NULL;
    dirs.start->count = 0;
    dirs.begin = dirs.end;
    files.start = malloc(sizeof(*dirs.start));
    files.start->next = NULL;
    files.start->count = 0;
    files.end = &files.start->next;
    files.begin = files.end;

    available = 8; // TODO adapt availability
    in_progress = 0;
}
void todo_destroy() {
    free(dirs.start);
    free(files.start);
}




const char* get_next(struct stat* info) {
    struct todo* got = gettodo_t_ptr(&dirs);
    if (!got) {
        got = gettodo_t_ptr(&files);
        if (!got) {
            return NULL;
        }
    }
    size_t n = strlen(got->path) + 1;
    char* ret = got->path;
    memcpy(info, &got->info, sizeof(*info));
    free(got);
    return ret;
}

void add_file(char* path, struct stat* info) {
    struct todo* data = malloc(sizeof(struct todo));
    memcpy(&data->info, info, sizeof(*info));
    data->path = path;
    puttodo_t_ptr(&files, data);
}
void add_dir(char* path, struct stat* info) {
    struct todo* data = malloc(sizeof(struct todo));
    memcpy(&data->info, info, sizeof(*info));
    data->path = path;
    puttodo_t_ptr(&dirs, data);
}
void add_path(const char* path) {
    struct stat info;
    if (stat(path, &info)) {
        perror(path);
        return;
    }
    size_t size = strlen(path);
    char* copy = malloc(size + 1);
    strncpy(copy, path, size + 1);
    mode_t mode = info.st_mode;
    if (S_ISREG(mode)) {
        // regular file
        add_file(copy, &info);

    } else if (S_ISDIR(mode)) {
        // directory
        add_dir(copy, &info);
    } else {
        // TODO what do?
        fprintf(stderr, "Dunno what to do with %s\n", path);
    }
}

static void start(void) {
    atomic_fetch_sub(&available, 1);
    atomic_fetch_add(&in_progress, 1);
}

void finish(void) {
    atomic_fetch_add(&available, 1);
    atomic_fetch_sub(&in_progress, 1);
}

void main_loop(void) {
     do {
        if (available) {
            struct stat info;
            const char* path = get_next(&info);
            if (path) {
                mode_t mode = info.st_mode;
                if (S_ISREG(mode)) {
                    file(path, &info);
                } else if (S_ISDIR(mode)) {
                    directory(path, mode);
                } else {
                    fprintf(stderr, "Dunno what to do with %s\n", path);
                }
                start();
            }
        }
        // TODO use futex or something instead
        sched_yield();
    } while (in_progress);
}
