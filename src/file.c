#include "file.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <aio.h>
#define flock _flock
#include <linux/fcntl.h>
#undef flock
#include <stdio.h>

#define BUF_SIZE (0x1000 * 20)
#define AIO_SIG SIGUSR1

struct job* file(char* path, struct stat* info) {
    int fd = open(path, O_NOATIME | O_NONBLOCK);
    if (fd == -1) {
        perror("path");
        goto cleanup;
    }
    int error;
    if (error = posix_fadvise(fd, 0, info->st_size, POSIX_FADV_SEQUENTIAL)) {
        errno = error;
        perror("posix_fadvise");
        goto cleanup;
    }
    /* Allocate the aio control block */
    struct aiocb* cb = malloc(sizeof(struct aiocb));
    /* Check the return value of malloc, in order to retain points */
    if(cb == NULL) {
        perror("malloc");
        goto cleanup;
    }
    struct job *aio_job = malloc(sizeof(struct job));
    if(aio_job == NULL) {
        perror("malloc");
        goto cleanup;
    }
    cb->aio_nbytes = BUF_SIZE;
    cb->aio_reqprio = 0;
    cb->aio_offset = 0;
    /* Single-threaded approach -> signals */
    cb->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    cb->aio_sigevent.sigev_signo = AIO_SIG;
    cb->aio_sigevent.sigev_value.sival_ptr = aio_job;

    aio_job->j_aiocb = cb;
    aio_job->j_filesz = info->st_size;

    return aio_job;
cleanup:
    if(cb != NULL)
        free(cb);
    if(aio_job != NULL)
        free(aio_job);
    close(fd);
    return NULL;
}
