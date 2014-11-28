#include "benchmarks.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static void make_level(
    const char* target,
    uint32_t depth,
    uint32_t width,
    uint32_t files,
    uint32_t file_size
) {
    if (chdir(target) == -1) {
        perror(target);
        return;
    }
    if (depth) {
        mode_t mode = 0775;
        for (size_t i = 0; i < width; i++) {
            char filename[18];
            snprintf(filename, sizeof(filename), "d%u", i);
            if (mkdir(filename, mode) == -1) {
                if (errno != EEXIST) {
                    perror(filename);
                    goto up;
                }
            }
            make_level(filename, depth - 1, width, files, file_size);
        }
    }
    FILE* rand = fopen("/dev/urandom", "r");
    if (rand == NULL) {
        perror("/dev/urandom");
        goto up;
    }
    for (size_t i = 0; i < files; i++) {
        char filename[17];
        snprintf(filename, sizeof(filename), "%u", i);
        FILE* f = fopen(filename, "w");
        if (f == NULL) {
            perror("fopen");
            goto up;
        }
        int fd = fileno(f);
        // if fallocate fails nbd
        posix_fallocate(fd, 0, file_size);
        const size_t chunk_size = 1000;
        char* buffer = malloc(chunk_size);
        for (size_t i = 0; i < file_size; i++) {
            // don't care if fread fails
            fread(buffer, 1, chunk_size, rand);
            size_t written = fwrite(buffer, 1, chunk_size, f);
            if (written < chunk_size) {
                perror(filename);
                goto cleanup;
            }
        }
        cleanup:
        free(buffer);
        fclose(f);
    }
    fclose(rand);
    up:
    chdir("..");
}

void setup(const char* benchmark_file, const char* target) {
    FILE* f = fopen(benchmark_file, "r");
    uint32_t depth, width, files, file_size;
    int scanned = fscanf(f, "%u %u %u %u", &depth, &width, &files, &file_size);
    if (scanned == EOF) {
        printf("%s", strerror(errno));;
        goto close;
    }
    if (scanned < 4) {
        printf("malformed file");
        goto close;
    }
    // TODO permissions
    mode_t mode = 0775;
    int mk = mkdir(target, mode);
    if (mk == -1) {
        if (errno != EEXIST) {
            perror(target);
            goto close;
        }
    }
    char cwd[300];
    const char* curr = getcwd(cwd,sizeof(cwd));
    if (curr == NULL) {
        perror("cwd");
    }
    make_level(target, depth, width, files, file_size);
    int ret = chdir(cwd);
    if(ret == -1) {
        perror("chdir");
        goto close;
    }

    close:
    fclose(f);
}
