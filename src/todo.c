#include "todo.h"
#include "file.h"

#include <errno.h>
#include <fcntl.h>
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

static struct mpsc {
    struct node {
        struct node* next;
        struct todo* data;
    } **begin, *end, **start;
} dirs, files;
void todo_init() {
    dirs.end = NULL;
    dirs.start = dirs.begin = &dirs.end;
    files.end = NULL;
    files.start = files.begin = &files.end;
}
static void put(struct mpsc* mpsc, struct todo* todo) {
    struct node* put = malloc(sizeof(struct node));
    put->data = todo;
    put->next = NULL;
    struct node** target = &mpsc->end;
    struct node* expected = NULL;
    while (!atomic_compare_exchange_strong(target, &expected, put)) {
        target = &expected->next;
        expected = NULL;
    }
}
void clean(struct mpsc* mpsc) {
    while (1) {
        struct node* to_free = *mpsc->start;
        if (!(to_free && to_free->next)) {
            break;
        }
        if (mpsc->begin != mpsc->start && mpsc->begin != &to_free->next) {
            mpsc->start = &to_free->next->next;
            free(to_free);
        } else {
            break;
        }
    }
}

static const char* get(struct mpsc* mpsc, struct stat* info) {
    if (*mpsc->begin) {
        struct node* got = *mpsc->begin;
        memcpy(info, &got->data->info, sizeof(*info));
        const char* ret = got->data->path;
        free(got->data);
        mpsc->begin = &got->next;
        clean(mpsc);
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
    mode_t mode = info.st_mode;
    if (S_ISREG(mode)) {
        // regular file
        add_file(path, &info);

    } else if (S_ISDIR(mode)) {
        // directory
        add_dir(path, &info);
    } else {
        // TODO what do?
        fprintf(stderr, "Dunno what to do with %s\n", path);
    }
}

