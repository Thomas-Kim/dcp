#include "directory.h"

#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <set>
using std::set;
#include <string>
using std::string;

set<string> in;
set<string> out;

volatile int finished = 0;

extern "C"
void add_path(const char* path) {
    out.insert(string(path));
}

extern "C"
void finish() {
    finished = 1;
}

void ls(char* dir) {
    int fds[2];
    if (pipe(fds) == -1) {
        perror("pipe");
        exit(errno);
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(errno);
    }
    if (pid) {
        // parent
        if (close(fds[1])) {
            perror("close");
            exit(errno);
        }
        int fd = fds[0];
        FILE* fp = fdopen(fd, "r");
        if (fp == NULL) {
            perror("fdopen");
            exit(errno);
        }
        // TODO get actual system max filename size
        const size_t MAX_FILENAME = 420;
        char* buffer = (char*) malloc(MAX_FILENAME);
        int test_dir_len = strlen(dir) + 1;
        int printed = snprintf(buffer, MAX_FILENAME, "%s/", dir);
        assert(printed == test_dir_len);
        while (1) {
            char* gets = fgets(buffer + test_dir_len, MAX_FILENAME, fp);
            if (gets == NULL) {
                if (errno) {
                    perror(gets);
                    exit(errno);
                } else {
                    // EOF
                    break;
                }
            }
            size_t len = strnlen(buffer, MAX_FILENAME);
            assert(len > test_dir_len);
            buffer[len - 1] = '\0'; // erase newline
            if (ignore(gets)) {
                continue;
            }
            in.insert(string(buffer));
        }
        if (fclose(fp)) {
            perror("fclose");
            exit(errno);
        }
    } else {
        // child
        if (close(1) == -1) {
            perror("stdout:close");
            // it is okay to have a closed stdout
            if (errno != EBADF) {
                exit(errno);
            }
        }
        if (close(fds[0]) == -1) {
            perror("pipein:close");
            exit(errno);
        }
        if (dup2(fds[1], 1) == -1) {
            perror("dup2");
            exit(errno);
        }
        if (close(fds[1]) == -1) {
            perror("pipeout:close");
            exit(errno);
        }
        // ls -1a
        char** argv = (char**) malloc(8 * sizeof(*argv));
        argv[0] = "find";
        argv[1] = "-1a";
        argv[2] = dir;
        argv[3] = NULL;
        execv("/bin/ls", argv);
        perror("execv");
        exit(errno);
    }
}

int main() {
    const size_t NUM_TESTS = 3;
    char* directories[NUM_TESTS] = {
        ".",
        "tst",
        ".."
    };
    for (size_t i = 0; i < NUM_TESTS; i++) {
        in.clear();
        out.clear();
        finished = 0;
        char* test_directory = directories[i];
        directory(test_directory);
        ls(test_directory);
        while (!finished) {
            sched_yield();
        }
        assert(out == in);
    }
    return 0;
}
