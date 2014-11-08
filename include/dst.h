#ifndef _DST
#define _DST

#include <sys/stat.h>

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

#ifdef __cplusplus
extern "C"
#endif
void get_dst_path(const char* path, char* buff);

// make a directory in a destination (blocking)
#ifdef __cplusplus
extern "C"
#endif
void mkdir_dst(const char* path, mode_t mode);

#endif
