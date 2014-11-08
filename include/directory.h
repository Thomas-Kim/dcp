// return whether we ignore an item (ie . or ..)

#include <sys/stat.h>

#ifdef __cplusplus
extern "C"
#endif
int ignore(const char* entry);

#ifdef __cplusplus
extern "C"
#endif
void directory(const char* path, mode_t mode);
