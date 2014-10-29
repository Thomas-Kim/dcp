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
#define AIO_SIGREAD SIGUSR1
#define AIO_SIGWRITE SIGUSR2

struct job* file(const char* path, struct stat* info) {
    int fd = open(path, O_NOATIME | O_NONBLOCK);
    if (fd == -1) {
        perror("path");
        return NULL;
    }
    int error;
    if (error = posix_fadvise(fd, 0, info->st_size, POSIX_FADV_SEQUENTIAL)) {
        errno = error;
        perror("posix_fadvise");
        return NULL;
    }
    /* Allocate the aio control block */
    struct aiocb* cb = malloc(sizeof(struct aiocb));
    /* Check the return value of malloc, in order to retain points */
    if(cb == NULL) {
        perror("malloc");
        goto abort;
    }
    struct job *aio_job = malloc(sizeof(struct job));
    if(aio_job == NULL) {
        perror("malloc");
        goto abort;
    }
    cb->aio_buf = malloc(BUF_SIZE);
    cb->aio_nbytes = BUF_SIZE;
    cb->aio_reqprio = 0;
    cb->aio_offset = 0;
    /* Single-threaded approach -> signals */
    cb->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    cb->aio_sigevent.sigev_value.sival_ptr = aio_job;

    aio_job->j_aiocb = cb;
    aio_job->j_filesz = info->st_size;

    return aio_job;
abort:
    if(aio_job != NULL)
        free(aio_job);
    if(cb != NULL)
        free(cb);
    close(fd);
    return NULL;
}

int job_schedule_read(struct job* aio_job) {
    aio_job->j_aiocb->aio_sigevent.sigev_signo = AIO_SIGREAD;
    int ret = aio_read(aio_job->j_aiocb);
    return ret;
}

int job_schedule_write(struct job* aio_job) {
    aio_job->j_aiocb->aio_sigevent.sigev_signo = AIO_SIGWRITE;
    int ret = aio_write(aio_job->j_aiocb);
    return ret;
}
