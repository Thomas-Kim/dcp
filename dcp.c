#include "todo.h"

#include <stdio.h>
static const char* USAGE ="\
usage: dcp SRC DST\n\
";
int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, USAGE);
        return -1;
    }
    char* src = argv[1];
    char* dst = argv[2];
    add_path(src);
    return 0;
}
