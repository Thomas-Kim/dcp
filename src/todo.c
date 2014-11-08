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

struct todo {
    struct stat info;
    const char* path;
    #ifdef __cplusplus
    // how to order items
    bool operator < (const todo& other) const {
        return info.st_size > other.info.st_size;
    };
    #endif // __cplusplus
};

// TODO restructure so that end is a pointer to and invalid node and not a pointer to a pointer
static struct mpsc {
    struct node {
        struct node* next;
        // strictly increasing count
        size_t count; // XXX overflow
        struct todo* data;
    } **begin, **end, *start;
} dirs, files;
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

    available = 4; // TODO adapt
    in_progress = 0;
}
void todo_destroy() {
    free(dirs.start);
    free(files.start);
}

// target must be a pointer to a "next" pointer
size_t count_from(struct node** target) {
    // the count follows the next ptr
    struct node* node = (struct node*) target;
    size_t prev_count = node->count;
    return prev_count;
}
static void update_tail(struct mpsc* mpsc, size_t count, struct node** proposed) {
    size_t proposed_count = count_from(proposed);
    while (1) {
        struct node** current_tail = mpsc->end;
        size_t current_count = count_from(current_tail);
        if (current_count > proposed_count) {
            return;
        }
        if (atomic_compare_exchange_strong(&mpsc->end, &current_tail, proposed)) {
            return;
        }
    }
}

static void put(struct mpsc* mpsc, struct todo* todo) {
    struct node* put = malloc(sizeof(struct node));
    put->data = todo;
    put->next = NULL;
    struct node** target;
    struct node* expected;
    try_node: // TODO restructure as loop
    target = mpsc->end;
    expected = NULL;
    put->count = count_from(target);
    if (!atomic_compare_exchange_strong(target, &expected, put)) {
        goto try_node;
    }
    update_tail(mpsc, put->count, &put->next);
}
static void clean_one(struct mpsc* mpsc) {
    struct node* to_free = mpsc->start;
    mpsc->start = to_free->next;
    free(to_free);
}

static const char* get(struct mpsc* mpsc, struct stat* info) {
    if (*mpsc->begin) {
        struct node* got = *mpsc->begin;
        memcpy(info, &got->data->info, sizeof(*info));
        const char* ret = got->data->path;
        free(got->data);
        mpsc->begin = &got->next;
        clean_one(mpsc);
        return ret;
    }
    return NULL;
}


const char* get_next(struct stat* info) {
    const char* ret = get(&dirs, info);
    if (ret) {
        return ret;
    }
    ret = get(&files, info);
    return ret;
}

void add_file(const char* path, struct stat* info) {
    struct todo* data = malloc(sizeof(struct todo));
    memcpy(&data->info, info, sizeof(*info));
    data->path = path;
    put(&files, data);
}
void add_dir(const char* path, struct stat* info) {
    struct todo* data = malloc(sizeof(struct todo));
    memcpy(&data->info, info, sizeof(*info));
    data->path = path;
    put(&dirs, data);
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
            } else {
                // check current work
            }
        }
        sched_yield();
    } while (in_progress);
}
