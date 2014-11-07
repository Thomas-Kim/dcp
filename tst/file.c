#include "file.h"

#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

volatile int finished = 0;
void finish() {
    finished = 1;
}

int main() {
    register_signal_handlers();
    set_src_root("tst/tmpsrc");
    set_dst_root("tst/tmpdst");
    
    struct stat info;
    if (lstat("tst/tmpsrc/0", &info) == -1) {
        perror("tst/tmpsrc/0");
        exit(errno);
    }
    file("tst/tmpsrc/0", &info);

    /* This should schedule a read, which
     * will schedule a write upon completion
     * etc. */
    // simulate main loop
    while (!finished) {
        sched_yield();
    }
    // ensure files equal using diff
    assert(WEXITSTATUS(system("diff tst/tmpsrc/0 tst/tmpdst/0")) == 0);
    return 0;
}
