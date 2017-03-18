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
#include "sendUtil.h"

#define INTERNAL_ERROR 500
#define QUEUE_SIZE 15

int main(int argc, char* argv[])
{
   /* Get port number from command line */
   unsigned short port = getPort(argc, argv);
   int socketFD;
   int newSocketFD;
   pid_t pid;

   /* Setup a signal handler that will wait for terminated child processes */
   signal_setup(SIGCHLD);


   if((socketFD = create_service(port, QUEUE_SIZE)) < 0)
   {
      fprintf(stderr, "Failed to create service to port %d\n", port);
      exit(-1);
   }

   while(1)
   {
      /* Accept new connection */
      if((newSocketFD = accept_connection(socketFD)) < 0)
      {
         fprintf(stderr, "accept_connection failure\n");
         exit(-1);
      }

      pid = fork();
      /* Fork failed, Internal Error */
      if(pid < 0)
      {
         sendError(INTERNAL_ERROR,newSocketFD);
         close(newSocketFD);
      }
      else
      {
         if(pid == 0)
         { /* Child - Client */
            close(socketFD);
            handle_request(newSocketFD);
         }
         else
         { /* Parent - Server */  
            close(newSocketFD);
         }
      }
   }

   return 0;
}
