#include <stdio.h>
#include "handle_request.h"
#include "readline.h"
#include <unistd.h>
#include <stdlib.h>

void handle_request(int socket)
{
   char* requestLine;
   
   /* TEST: Set fd socket to a FILE*, so that we can use readline */
   /* Which can read a line with no byte limitations. */
   FILE* client = fdopen(socket, "r+b");
   
   /* Check if fdopen worked */
   if(client == NULL)
   {
      perror("");
      exit(-1);
   }

   /* Receive */
   requestLine = readline(client);
   printf("Request was: %s\n", requestLine);

   close(socket);
   exit(0);
}
