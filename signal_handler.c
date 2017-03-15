#include "signal_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

void signal_setup(int signo)
{
   struct sigaction action;

   if(sigemptyset(&action.sa_mask) == -1)
   {
      perror("sigemptyset");
      exit(-1);
   }
   action.sa_flags = 0;
   action.sa_handler = signal_handler;

   if(sigaction(signo, &action, NULL) == -1)
   {
      perror("sigaction");
      exit(-1);
   }
}

void signal_handler(int signo)
{  
   if(signo == SIGCHLD)
   {
      while(waitpid(-1, NULL, WNOHANG) > 0);
   }
}
