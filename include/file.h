#include <sys/stat.h>

#include <sys/types.h>
struct job {
    size_t j_filesz;
    struct aiocb* j_aiocb;
};

#ifdef __cplusplus
extern "C"
#endif
struct job* file(char* path, struct stat* info);
