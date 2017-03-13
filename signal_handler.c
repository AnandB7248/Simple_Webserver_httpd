#include "signal_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

void sigchld_setup(int signo)
{
   struct sigaction action;

   if(sigemptyset(&action.sa_mask) == -1)
   {
      perror("sigemptyset");
      exit(-1);
   }
   action.sa_flags = 0;
   action.sa_handler = handle_sigchld;

   if(sigaction(signo, &action, NULL) == -1)
   {
      perror("sigaction");
      exit(-1);
   }
}

void handle_sigchld(int signo)
{  
   /* Signal */
   if(signo == SIGCHLD)
   {
      while(waitpid(-1, NULL, WNOHANG) > 0);
   }
}
