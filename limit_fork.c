#include "limit_fork.h"
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>

void limit_fork(rlim_t max_procs)
{
   struct rlimit rl;

   if (getrlimit(RLIMIT_NPROC, &rl))
   {
      perror("getrlimit");
      exit(-1);
   }

   rl.rlim_cur = max_procs;

   if (setrlimit(RLIMIT_NPROC, &rl))
   {
      perror("setrlimit");
      exit(-1);
   }
}
