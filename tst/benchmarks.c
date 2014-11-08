#include "benchmarks.h"
#include "equals.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SRC "tst/tmpsrc"
#define DST "tst/tmpdst"
#define PATH "bin/"
#define CMD "dcp"
static const char* const test_src = SRC;
static const char* const test_dst = DST;
static const char* const pathed_cmd = PATH CMD;
static const char* const cmd = CMD;
char* const argv[4] = {
    CMD,
    SRC,
    DST,
    NULL
};

static void run_benchmark(const char* benchmark_file, const char* name) {
    struct stat info;
    if (stat(benchmark_file, &info) == -1) {
        perror(benchmark_file);
        return;
    }
    if (!S_ISREG(info.st_mode)) {
        return;
    }
    printf(name);
    putchar(':');
    putchar('\t');
    setup(benchmark_file, test_src);
    int status;
    struct tms interval[2];
    if (times(&interval[0]) == -1) {
        perror("times");
    }
    int pid = fork();
    switch (pid) {
    case -1:
        perror("fork");
        exit(errno);
    case 0:
        execv(pathed_cmd, argv);
        perror("exec");
        exit(errno);
    default:
        pid = waitpid(pid, &status, 0);
        if (times(&interval[1]) == -1) {
            perror("times");
        }
        break;
    }
    clock_t elapsed = (interval[1].tms_cutime - interval[0].tms_cutime)
                    + (interval[1].tms_cstime - interval[0].tms_cstime);
    printf("%u", elapsed);
    
    putchar('\n');
}

int main(int argc, char* argv[]) {
    const char* benchmarks = "tst/benchmarks/";
    putchar('\n');
    DIR* d = opendir(benchmarks);
    if (d == NULL) {
        perror(benchmarks);
        exit(errno);
    }
    struct dirent* ent = readdir(d);
    while (ent) {
        char full_path[300];
        char* start = stpncpy(full_path, benchmarks, 50);
        char* name = ent->d_name;
        start = stpncpy(start, name, 250);
        run_benchmark(full_path, name);
        ent = readdir(d);
    }
    closedir(d);
    return 0;
}
