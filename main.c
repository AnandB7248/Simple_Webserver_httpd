#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "simple_net.h"
#include "getPort.h"

#define QUEUE_SIZE 15

int main(int argc, char* argv[])
{
   unsigned short port = getPort(argc, argv);
   int socket = create_service(port, QUEUE_SIZE);
   printf("socket: %d\n", socket);   
   if(socket == -1)
   {
      fprintf(stderr, "Failed to create service to port: %d\n", port);
      exit(-1);   
   }

   return 0;
}
