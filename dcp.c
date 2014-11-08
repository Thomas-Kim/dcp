#include "todo.h"
#include "file.h"

#include <stdio.h>
static const char* USAGE ="\
usage: dcp SRC DST\n\
";
int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "%s", USAGE);
        return -1;
    }
    const char* src = argv[1];
    const char* dst = argv[2];
    set_src_root(src);
    set_dst_root(dst);
    register_signal_handlers();
    todo_init();
    add_path(src);
    main_loop();
    todo_destroy();
    return 0;
}
