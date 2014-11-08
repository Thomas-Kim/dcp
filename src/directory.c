#include "directory.h"
#include "todo.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#define flock _flock
#include <linux/fcntl.h>
#undef flock
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

void* do_directory(void* arg) {
    const char* path = arg;
    int fd = open(path, O_NOATIME, O_DIRECTORY);
    if (fd == -1) {
        perror(path);
        exit(errno);
    }
    DIR* dp = fdopendir(fd);
    if (dp == NULL) {
        perror(path);
        exit(errno);
    }
    struct dirent* ent;
    char *full_path = malloc(200);
    do {
        ent = readdir(dp);
        if (ent == NULL) {
            if (errno) {
                perror(path);
                return NULL;
            }
            continue;
        }
        snprintf(full_path, 200, "%s/%s", path, ent->d_name);
        add_path(full_path);
    } while(ent != NULL);
    free(full_path);
    finish();
    return NULL;
}

void directory(const char* path) {
    pthread_t thread;
    // TODO copy the path?
    pthread_create(&thread, NULL,  do_directory, (void*) path);
    pthread_detach(thread);
}
