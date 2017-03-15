#include "getPort.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define PORT_LOW_BOUND 1024
#define PORT_UPP_BOUND 65535

int getPort(int argc, char* argv[])
{
   int port;

   if(argc != 2)
   {
      fprintf(stderr, "usage: httpd [port] \n");
      exit(-1);
   }

   /* Convert command line argument to an integer data type */
   port = atoi(argv[1]);

   /* If argv[1] could not be converted to a integer data type */
   if(port == 0) 
   {
      fprintf(stderr, "usage: httpd [port] \n");
      fprintf(stderr, "Please input an integer value for [port]\n");
      exit(-1);
   }
   /* If port value given is negative */
   else if(port < 0)
   {
      fprintf(stderr, "usage: httpd [port] \n");
      fprintf(stderr, "Error: [port] must be a positive number\n");  
      exit(-1); 
   }
   /* Port should be between 1024 and 65535 */
   else if((port < PORT_LOW_BOUND) || (port > PORT_UPP_BOUND))
   {
      fprintf(stderr, "usage: httpd [port] \n");
      fprintf(stderr, "Error: [port] should be a value between 1024 and 65535\n");
      exit(-1);
   }

   return port;
}
