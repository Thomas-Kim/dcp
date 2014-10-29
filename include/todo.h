#include <sys/stat.h>

// get the next work todo, set *info to path's stat info if info not NULL
// if nothing todo, return NULL
#ifdef __cplusplus
extern "C"
#endif
char* get_next(struct stat* info);

// add work todo
#ifdef __cplusplus
extern "C"
#endif
void add_path(char* p);

// Pop the next job off the file queue and execute it
#ifdef __cplusplus
extern "C"
#endif
void do_file(void);

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
