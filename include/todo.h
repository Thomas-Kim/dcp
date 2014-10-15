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
