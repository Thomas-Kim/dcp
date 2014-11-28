#define _GNU_SOURCE

#include "file.h"
#include "dst.h"


#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define BUF_SIZE (0x1000 * 80)
#define STRBUF_SIZE 0x400
#define AIO_SIGREAD SIGUSR1
#define AIO_SIGWRITE SIGUSR2

// defined by test or by todo
void finish();

struct job {
    size_t j_filesz;
    int src_fd, dst_fd;
    struct aiocb* j_aiocb;
    const char* path;
};

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

static void sayf(FILE* stream, const char* format, ...) {
    va_list list;
    va_start(list, format);
    pthread_mutex_lock(&print_mutex);
    vfprintf(stream, format, list);
    pthread_mutex_unlock(&print_mutex);
}

static int job_schedule_write(struct job* aio_job);
static void aio_sigread_handler(union sigval sigval) {
    struct job* aio_job = sigval.sival_ptr;
    ssize_t ahead = readahead(aio_job->src_fd, aio_job->j_aiocb->aio_offset + BUF_SIZE, BUF_SIZE);
    if (ahead == -1) {
        perror("readahead");
        exit(errno);
    }
    job_schedule_write(aio_job);
}
static int job_schedule_read(struct job* aio_job);
static void aio_sigwrite_handler(union sigval sigval) {
    struct job* aio_job = sigval.sival_ptr;
    aio_job->j_aiocb->aio_offset += BUF_SIZE;
    if(aio_job->j_aiocb->aio_offset < aio_job->j_filesz) {
        job_schedule_read(aio_job);
    }
    else {
        close(aio_job->src_fd);
        close(aio_job->dst_fd);
        //sayf(stderr, "FINISH %s\n", aio_job->path);
        finish();
    }
}

static int job_schedule_read(struct job* aio_job) {
    //sayf(stderr, "read   %s\n", aio_job->path);
    aio_job->j_aiocb->aio_sigevent.sigev_notify_function = aio_sigread_handler;
    aio_job->j_aiocb->aio_sigevent.sigev_value.sival_ptr = aio_job;
    aio_job->j_aiocb->aio_fildes = aio_job->src_fd;
    // TODO calculate size
    off_t ending_position = aio_job->j_aiocb->aio_offset + BUF_SIZE;
    if(ending_position > aio_job->j_filesz)
        aio_job->j_aiocb->aio_nbytes = aio_job->j_filesz - aio_job->j_aiocb->aio_offset;
    else
        aio_job->j_aiocb->aio_nbytes = BUF_SIZE;
    int ret = aio_read(aio_job->j_aiocb);
    if(ret == -1) {
        perror("aio_read");
    }
    return ret;
}

int job_schedule_write(struct job* aio_job) {
    //sayf(stderr, "write  %s\n", aio_job->path);
    aio_job->j_aiocb->aio_sigevent.sigev_notify_function = aio_sigwrite_handler;
    aio_job->j_aiocb->aio_fildes = aio_job->dst_fd;
    // TODO calculate size
    off_t ending_position = aio_job->j_aiocb->aio_offset + BUF_SIZE;
    if(ending_position > aio_job->j_filesz)
        aio_job->j_aiocb->aio_nbytes = aio_job->j_filesz - aio_job->j_aiocb->aio_offset;
    else
        aio_job->j_aiocb->aio_nbytes = BUF_SIZE;
    int ret = aio_write(aio_job->j_aiocb);
    if(ret == -1) {
        perror("aio_read");
    }
    return ret;
}

void file(const char* path, struct stat* info) {
    int fd = open(path, O_NOATIME | O_NONBLOCK | O_RDONLY);
    if (fd == -1) {
        perror(path);
        return;
    }
    if (errno = posix_fadvise(fd, 0, info->st_size, POSIX_FADV_SEQUENTIAL)) {
        perror("posix_fadvise");
        goto abort_close;
    }
    char buf[STRBUF_SIZE];
    get_dst_path(path, buf);
    int dfd = open(buf, O_NOATIME | O_NONBLOCK | O_WRONLY | O_CREAT, info->st_mode);
    if(dfd == -1) {
        perror(buf);
        goto abort_close;
    }
    /* Allocate the aio control block */
    struct aiocb* cb = malloc(sizeof(struct aiocb));
    /* Check the return value of malloc, in order to retain points */
    if(cb == NULL) {
        perror("malloc");
        goto abort_close2;
    }
    struct job *aio_job = malloc(sizeof(struct job));
    if(aio_job == NULL) {
        perror("malloc");
        goto abort;
    }
    bzero(cb, sizeof(*cb));
    cb->aio_buf = malloc(BUF_SIZE);
    if (cb->aio_buf == NULL) {
        perror("malloc");
        goto abort;
    }
    cb->aio_nbytes = BUF_SIZE;
    cb->aio_reqprio = 0;
    cb->aio_offset = 0;
    /* Single-threaded approach -> signals */
    cb->aio_sigevent.sigev_notify = SIGEV_THREAD;
    cb->aio_sigevent.sigev_value.sival_ptr = aio_job;

    aio_job->j_aiocb = cb;
    aio_job->j_filesz = info->st_size;
    aio_job->src_fd = fd;
    aio_job->dst_fd = dfd;
    aio_job->path = path;

    job_schedule_read(aio_job);
    return;
abort:
    if(aio_job != NULL) {
        free(aio_job);
    }
    if(cb != NULL) {
        free(cb);
    }
abort_close2:
    close(dfd);
abort_close:
    close(fd);
    //sayf(stderr, "FINISH %s\n", path);
    finish();
    return;
}

int register_signal_handlers(void) {
    return 0;
}
