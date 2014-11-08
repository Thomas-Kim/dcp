#include "dst.h"

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
