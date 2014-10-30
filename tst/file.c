#include "file.h"

#include <assert.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    register_signal_handlers();
    set_src_root("tst/tmpsrc");
    set_dst_root("tst/tmpdst");
    
    struct stat info;
    lstat("tst/tmpsrc/0", &info);
    struct job* aio_job = file("tst/tmpsrc/0", &info);

    /* This should schedule a read, which
     * will schedule a write upon completion
     * etc. */
    job_schedule_read(aio_job);
    // simulate main loop
    while (1) {
        if (aio_job->dst_fd) {
            sched_yield();
        } else {
            exit(0);
        }
    }
    return 0;
}
