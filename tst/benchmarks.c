#include "benchmarks.h"
#include "equals.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SRC "tst/tmpsrc"
#define DST "tst/tmpdst"
#define PATH "bin/"
#define DCP "dcp"
static const char* const test_src = SRC;
static const char* const test_dst = DST;
static const char* const pathed_dcp = PATH DCP;
static const char* const pathed_cpr = "/bin/cp";
char* const dcp_argv[4] = {
    DCP,
    SRC,
    DST,
    NULL
};
char* const cpr_argv[5] = {
    "cp",
    "-r",
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
    system("rm -rf " SRC);
    setup(benchmark_file, test_src);
    for (int i = 0; i < 2; i++) {
        // set up work
        system("rm -rf " DST);
        // pick command
        const char* pathed_cmd;
        char *const * argv;
        switch (i) {
            case 0:
                pathed_cmd = pathed_dcp;
                argv = dcp_argv;
                break;
            case 1:
                pathed_cmd = pathed_cpr;
                argv = cpr_argv;
                break;
            default:
                fprintf(stderr, "BAD ITERATION %i\n", i);
                exit(EINVAL);
        }
        int status;
        struct timeval interval[2];
        if (gettimeofday(&interval[0], NULL) == -1) {
            perror("gettimeofday");
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
            if (gettimeofday(&interval[1], NULL) == -1) {
                perror("times");
            }
            break;
        }
        time_t elapsed = (interval[1].tv_usec - interval[0].tv_usec)
                        + 1000000 * (interval[1].tv_sec - interval[0].tv_sec);
        printf("\t%u", elapsed);
    }
    
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
