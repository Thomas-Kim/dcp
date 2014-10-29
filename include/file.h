#include <sys/stat.h>
#include <signal.h>

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
#endif
struct job* file(const char* path, struct stat* info);


#ifdef __cplusplus
extern "C"
#endif
int register_signal_handlers(void);

#if 0
#ifdef __cplusplus
extern "C"
#endif
int job_schedule_read(struct job* aio_job);

#ifdef __cplusplus
extern "C"
#endif
int job_schedule_write(struct job* aio_job);

#ifdef __cplusplus
extern "C"
#endif
void aio_sigread_handler(int signo, siginfo_t* si, void* ucontext);

#ifdef __cplusplus
extern "C"
#endif
void aio_sigwrite_handler(int signo, siginfo_t* si, void* ucontext);
#endif
