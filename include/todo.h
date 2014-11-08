#include "directory.h"
#include "dst.h"
#include "file.h"

#include <sys/stat.h>

// get the next work todo, set *info to path's stat info if info not NULL
// if nothing todo, return NULL
#ifdef __cplusplus
extern "C"
#endif
const char* get_next(struct stat* info);

// add work todo
// you will receive the same char* back that you put when you call get_next
#ifdef __cplusplus
extern "C"
#endif
void add_path(const char* p);

// cleanup todo structures
#ifdef __cplusplus
extern "C"
#endif
void todo_destroy(void);

// initialize todo structures
#ifdef __cplusplus
extern "C"
#endif
void todo_init(void);

// Note that you have finished a unit of work
#ifdef __cplusplus
extern "C"
#endif
void finish(void);

#ifdef __cplusplus
extern "C"
#endif
void main_loop(void);
