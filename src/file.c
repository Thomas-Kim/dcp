#include "file.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <aio.h>
#include <string.h>
#define flock _flock
#include <linux/fcntl.h>
#undef flock
#include <stdio.h>

#include <assert.h>

#define BUF_SIZE (0x1000 * 20)
#define STRBUF_SIZE 0x400
#define AIO_SIGREAD SIGUSR1
#define AIO_SIGWRITE SIGUSR2

static char dst_root[STRBUF_SIZE];
static char src_root[STRBUF_SIZE];

struct job* file(const char* path, struct stat* info) {
    int fd = open(path, O_NOATIME | O_NONBLOCK | O_RDONLY);
    if (fd == -1) {
        perror("path");
        return NULL;
    }
    int error;
    if (error = posix_fadvise(fd, 0, info->st_size, POSIX_FADV_SEQUENTIAL)) {
        errno = error;
        perror("posix_fadvise");
        goto abort;
    }
    char buf[STRBUF_SIZE];
    get_dst_path(path, buf);
    int dfd = open(buf, O_NOATIME | O_NONBLOCK | O_WRONLY | O_CREAT, info->st_mode);
    if(dfd == -1) {
        perror(buf);
        goto abort;
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
    aio_job->src_fd = fd;
    aio_job->dst_fd = dfd;

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
    printf("read\n");
    aio_job->j_aiocb->aio_sigevent.sigev_signo = AIO_SIGREAD;
    aio_job->j_aiocb->aio_fildes = aio_job->src_fd;
    // TODO calculate size
    aio_job->j_aiocb->aio_nbytes = aio_job->j_filesz < BUF_SIZE ? aio_job->j_filesz : BUF_SIZE;
    int ret = aio_read(aio_job->j_aiocb);
    if(ret == -1)
        perror("aio_read");
    return ret;
}

int job_schedule_write(struct job* aio_job) {
    printf("write\n");
    aio_job->j_aiocb->aio_sigevent.sigev_signo = AIO_SIGWRITE;
    aio_job->j_aiocb->aio_fildes = aio_job->dst_fd;
    // TODO calculate size
    aio_job->j_aiocb->aio_nbytes = aio_job->j_filesz < BUF_SIZE ? aio_job->j_filesz : BUF_SIZE;
    int ret = aio_write(aio_job->j_aiocb);
    if(ret == -1)
        perror("aio_read");
    return ret;
}

static void aio_sigread_handler(int signo, siginfo_t* si, void* ucontext) {
    struct job* aio_job = si->si_value.sival_ptr;
    job_schedule_write(aio_job);
}

static void aio_sigwrite_handler(int signo, siginfo_t* si, void* ucontext) {
    struct job* aio_job = si->si_value.sival_ptr;
    aio_job->j_aiocb->aio_offset += BUF_SIZE;
    if(aio_job->j_aiocb->aio_offset < aio_job->j_filesz) {
        job_schedule_read(aio_job);
    }
    else {
        close(aio_job->src_fd);
        close(aio_job->dst_fd);
        // FIXME find a better way to indicate done
        aio_job->src_fd = 0;
        aio_job->dst_fd = 0;
    }
}

int register_signal_handlers(void) {
    struct sigaction write_action, read_action;
    write_action.sa_sigaction = aio_sigwrite_handler;
    write_action.sa_flags = SA_SIGINFO;
    read_action.sa_sigaction = aio_sigread_handler;
    read_action.sa_flags = SA_SIGINFO;
    int ret = sigaction(AIO_SIGWRITE, &write_action, NULL);
    if(ret < 0)
        perror("sigaction");
    ret = sigaction(AIO_SIGREAD, &read_action, NULL);
    if(ret < 0)
        perror("sigaction");
    return 0;
}

void get_dst_path(const char* path, char* buff) {
    strcpy(buff, dst_root);
    strcpy(&buff[strlen(dst_root)], &(path[strlen(src_root)]));
}

void set_src_root(const char* root) {
    strcpy(src_root, root);
}

void set_dst_root(const char* root) {
    strcpy(dst_root, root);
}
