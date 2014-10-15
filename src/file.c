#include "file.h"

#include <errno.h>
#include <fcntl.h>
#define flock _flock
#include <linux/fcntl.h>
#undef flock
#include <stdio.h>

void file(char* path, struct stat* info) {
    int fd = open(path, O_NOATIME | O_NONBLOCK);
    if (fd == -1) {
        perror("path");
        return;
    }
    int error;
    if (error = posix_fadvise(fd, 0, info->st_size, POSIX_FADV_SEQUENTIAL)) {
        errno = error;
        perror("posix_fadvise");
    }
}
