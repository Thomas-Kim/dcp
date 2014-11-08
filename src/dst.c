#include "dst.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRBUF_SIZE 0x400

static char dst_root[STRBUF_SIZE];
static char src_root[STRBUF_SIZE];

void get_dst_path(const char* path, char* buff) {
    strcpy(buff, dst_root);
    strcpy(&buff[strlen(dst_root)], &path[strlen(src_root)]);
}

void set_src_root(const char* root) {
    strcpy(src_root, root);
}

void set_dst_root(const char* root) {
    strcpy(dst_root, root);
}

void mkdir_dst(const char* path, mode_t mode) {
    char* dst = malloc(STRBUF_SIZE);
    get_dst_path(path, dst);
    if (mkdir(dst, mode) == -1) {
        switch (errno) {
        case EEXIST:
            // ignore these
            break;
        default:
            perror(dst);
            exit(errno);
        }
    }
    free(dst);
}
