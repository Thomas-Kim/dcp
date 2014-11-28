#include "benchmarks.h"

#include <errno.h>
#include <stdio.h>
const char* USAGE = "\
bin/setup FILE PATH\n\
";
int main(int argc, char** argv) {
    if (argc < 3) {
        fputs(USAGE, stderr);
        return EINVAL;
    }
    setup(argv[1], argv[2]);
    return 0;
}
