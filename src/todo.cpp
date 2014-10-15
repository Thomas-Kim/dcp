#include "todo.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


#include <queue>
using std::priority_queue;

struct todo {
    struct stat info;
    char* path;
    // how to order items
    bool operator < (const todo& other) const {
        return info.st_size > other.info.st_size;
    };
};

static priority_queue<todo> files;
static priority_queue<todo> directories;

char* get_next(struct stat* info) {
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

void add_file(char* path, struct stat* info) {
    struct todo put;
    memcpy(&put.info, info, sizeof(*info));
    put.path = path;
    files.push(put);
}
void add_dir(char* path, struct stat* info) {
    struct todo put;
    memcpy(&put.info, info, sizeof(*info));
    put.path = path;
    directories.push(put);
}
void add_path(char* path) {
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

