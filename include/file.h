#include <sys/stat.h>
#include <signal.h>

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
#endif
void file(const char* path, struct stat* info);


#ifdef __cplusplus
extern "C"
#endif
int register_signal_handlers(void);

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
