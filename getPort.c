#include "getPort.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0

unsigned short getPort(int argc, char* argv[])
{
   unsigned short status;

   if(argc != 2)
   {
      fprintf(stderr, "usage: httpd [port] \n");
      exit(-1);
   }

   status = isPosNum(argv[1]);

   if(status == FALSE)
   {
      fprintf(stderr, "Error: [port] must be a positive number \n");  
      exit(-1); 
   }
    
   return atoi(argv[1]);
}

unsigned short isPosNum(char* argument)
{
   int length;
   int i = 0;

   if(argument == NULL)
      return FALSE;
   
   length = strlen(argument);

   if(argument[0] == '-')
   {
      return FALSE;
   }
   
   for(i = 1; i < length; i++)
   {
      if(!isdigit(argument[i]))
         return FALSE;
   }
   
   return TRUE;
}
