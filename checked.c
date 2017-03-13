#include "checked.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int checked_fork()
{
   pid_t tmp;

   if((tmp = fork()) < 0)
   {
      perror("Failed to fork: ");
      exit(-1);
   }

   return tmp;
}

int checked_open(char* path, int oflags)
{
   int fd;

   if((fd = open(path, oflags)) < 0)
   {
      perror("Open failed!");
      exit(-1);
   }

   return fd;
}

void checked_close(int fd)
{
   if(close(fd) < 0)
   {
      fprintf(stderr, "close failed!\n");
      exit(-1);
   }
}
