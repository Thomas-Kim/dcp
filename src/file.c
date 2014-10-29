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

#include <assert.h>

#define BUF_SIZE (0x1000 * 20)
#define AIO_SIGREAD SIGUSR1
#define AIO_SIGWRITE SIGUSR2

struct job {
    size_t j_filesz;
    struct aiocb* j_aiocb;
};

struct job* file(char* path, struct stat* info) {
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
    cb->aio_fildes = fd;
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

static void aio_sigread_handler(int signo, siginfo_t* si, void* ucontext) {
    struct job* aio_job = si->si_value.sival_ptr;
    job_schedule_write(aio_job);
}

static void aio_sigwrite_handler(int signo, siginfo_t* si, void* ucontext) {
    struct job* aio_job = si->si_value.sival_ptr;
    aio_job->j_aiocb->aio_offset += BUF_SIZE;
    job_schedule_read(aio_job);
}

int register_signal_handlers(void) {
    struct sigaction write_action, read_action;
    write_action.sa_sigaction = aio_sigwrite_handler;
    write_action.sa_flags = SA_SIGINFO;
    read_action.sa_sigaction = aio_sigread_handler;
    read_action.sa_flags = SA_SIGINFO;
    sigaction(AIO_SIGWRITE, &write_action, NULL);
    sigaction(AIO_SIGREAD, &read_action, NULL);
    return 0;
}
