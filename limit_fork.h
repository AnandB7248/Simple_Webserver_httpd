#ifndef LIMIT_FORK
#define LIMIT_FORK

#include <sys/resource.h>

void limit_fork(rlim_t max_procs);

#endif

