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

// Set the root of the source file tree
#ifdef __cplusplus
extern "C"
#endif
void set_src_root(const char * root);

// Set the root of the destination file tree
#ifdef __cplusplus
extern "C"
#endif
void set_dst_root(const char * root);

// Note that you have finished a unit of work
#ifdef __cplusplus
extern "C"
#endif
void finish(void);

#ifdef __cplusplus
extern "C"
#endif
void main_loop(void);
