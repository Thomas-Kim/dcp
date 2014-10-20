#include "equals.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

const size_t buf_size = 1000;
int filesEqual(const char* one, const char* two) {
    FILE* f1 = fopen(one, "r");
    if (f1 == NULL) {
        perror(one);
        exit(errno);
    }
    FILE* f2 = fopen(two, "r");
    if (f2 == NULL) {
        perror(two);
        exit(errno);
    }
    size_t r1, r2;
    char* buff1 = malloc(buf_size);
    char* buff2 = malloc(buf_size);
    if (!buff1 || !buff2) {
        perror("malloc");
        exit(ENOMEM);
    }
    do {
        r1 = fread(buff1, 1, buf_size, f1);
        r2 = fread(buff2, 1, buf_size, f2);
        if (r1 != r2) {
            return false;
        }
        if (bcmp(buff1, buff2, r1) != 0) {
            return false;
        }
    } while (r1 & r2);
    free(buff1);
    free(buff2);
    fclose(f1);
    fclose(f2);
    return true;
}
int dirsEqual(const char* one, const char* two) {
    DIR* d1 = opendir(one);
    if (d1 == NULL) {
        perror(one);
        exit(errno);
    }
    DIR* d2 = opendir(two);
    if (d2 == NULL) {
        perror(two);
        exit(errno);
    }
    struct dirent *e1, *e2;
    do {
        e1 = readdir(d1);
        e2 = readdir(d2);
        // FIXME file order varies
        if (strcmp(e1->d_name, e2->d_name) != 0) {
            return 0;
        }
    } while (e1 && e2);
    if (e1 || e2) {
        return false;
    }
    closedir(d1);
    closedir(d2);
    return true;
}
void assertDirsEqual(const char* one, const char* two) {
    assert(dirsEqual(one, two));
}
void assertFilesEqual(const char* one, const char* two) {
    assert(filesEqual(one, two));
}
