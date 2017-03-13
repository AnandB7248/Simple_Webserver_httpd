#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "simple_net.h"
#include "getPort.h"
#include "checked.h"
#include "handle_request.h"
#include "signal_handler.h"
#include "limit_fork.h"

#define QUEUE_SIZE 15
#define _XOPEN_SOURCE

int main(int argc, char* argv[])
{
   /* Get port number from command line */
   unsigned short port = getPort(argc, argv);  
   int socketFD = create_service(port, QUEUE_SIZE);
   int newSocketFD;
   pid_t pid;

   limit_fork(25);
   sigchld_setup(SIGCHLD);

   while(1)
   {
      /* Accept new connection */
      if((newSocketFD = accept_connection(socketFD)) < 0)
      {
         fprintf(stderr, "Fail to accept connection.\n");
         exit(-1);
      }

      /* Handle connection request */
      pid = checked_fork();

      if(pid == 0)
      { /* Child Process - Client */
         close(socketFD);
         handle_request(newSocketFD); /* Up to the child to eventually terminate and clean up resources */
      }
      else
      { /* Parent Process */
         close(newSocketFD);
      }
   }

   return 0;
}
