#include "directory.h"

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
}

void directory(const char* path) {
    pthread_t thread;
    pthread_create(&thread, NULL,  do_directory, path);
    pthread_detach(thread);
}
