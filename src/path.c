#include "directory.h"
#include "file.h"
#include "path.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void path(char* pth) {
    struct stat info;
    if (stat(pth, &info)) {
        perror("stat");
        exit(errno);
    }
    if (S_ISREG(info.st_mode)) {
        // regular file
        file(pth);

    } else if (S_ISDIR(info.st_mode)) {
        // directory
        directory(pth);
    } else {
        // TODO what do?
    }
}
