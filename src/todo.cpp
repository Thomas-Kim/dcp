#include "todo.h"
#include "file.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <queue>
using std::priority_queue;

const size_t buf_size = 0x400; // Buffer size for paths

struct todo {
    struct stat info;
    const char* path;
    // how to order items
    bool operator < (const todo& other) const {
        return info.st_size > other.info.st_size;
    };
};

static priority_queue<todo> files;
static priority_queue<todo> directories;
static char dst_root[buf_size];
static char src_root[buf_size];

const char* get_next(struct stat* info) {
    struct todo ret;
    if (!files.empty()) {
        ret = files.top();
        files.pop();
    } else if (!directories.empty()) {
        ret = directories.top();
        directories.pop();
    } else {
        return NULL;
    }
    if (info) {
        memcpy(info, &ret.info, sizeof(*info));
    }
    return ret.path;
}

void add_file(const char* path, struct stat* info) {
    struct todo put;
    memcpy(&put.info, info, sizeof(*info));
    put.path = path;
    files.push(put);
}
void add_dir(const char* path, struct stat* info) {
    struct todo put;
    memcpy(&put.info, info, sizeof(*info));
    put.path = path;
    directories.push(put);
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
void get_dst_path(const char* path, char* buff) {
    strcpy(buff, dst_root);
    strcpy(&buff[strlen(dst_root)], &(path[strlen(src_root) - 1]));
}
void do_file() {
    struct stat info;
    const char* src_path = get_next(&info);

    char dst_path[buf_size];
    get_dst_path(src_path, dst_path);
    // Open the file using file(2) from file.h
    struct job* aio_job = file(src_path, &info);
    /* Start the job using AIO interface */
}
void set_src_root(const char* root) {
    strcpy(src_root, root);
}
void set_dst_root(const char* root) {
    strcpy(dst_root, root);
}
