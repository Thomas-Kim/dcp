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
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int ignore(const char* item) {
    return !strcmp(item, ".")
        || !strcmp(item, "..");
}

void* do_directory(void* arg) {
    const char* path = arg;
    int fd = open(path, O_NOATIME, O_DIRECTORY);
    if (fd == -1) {
        perror(path);
        goto end;
    }
    DIR* dp = fdopendir(fd);
    if (dp == NULL) {
        perror(path);
        close(fd);
        goto end;
    }
    struct dirent* ent;
    char *full_path = malloc(200);
    do {
        ent = readdir(dp);
        if (ent == NULL) {
            if (errno) {
                perror(path);
                goto end;
            }
            continue;
        }
        if (ignore(ent->d_name)) {
            continue;
        }
        snprintf(full_path, 200, "%s/%s", path, ent->d_name);
        add_path(full_path);
    } while(ent != NULL);
    free(full_path);
    closedir(dp);
    end:
    finish();
    return NULL;
}

void directory(const char* path) {
    pthread_t thread;
    // TODO copy the path?
    pthread_create(&thread, NULL,  do_directory, (void*) path);
    pthread_detach(thread);
}
