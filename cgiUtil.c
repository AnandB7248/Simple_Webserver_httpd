#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "cgiUtil.h"
#include "handle_request.h"

#define TRUE 1
#define FALSE 0
#define MAX 101

int execCgiCmd(char* filename, char** nameOfNewTmpFile)
{
   /* By the way this program is written, filename argument is guaranteed to be: ./cgi-like/(contents) */

   /* If cmd has arguments */
   if(cgiContainsArgs(filename))
      return execCgiWithArgs(filename, nameOfNewTmpFile);
   /* If cmd has no arguments */
   else
      return execCgiNoArgs(filename, nameOfNewTmpFile);
}

int execCgiNoArgs(char* cmd, char** nameOfNewTmpFile)
{
   pid_t pid;
   int tmpFD;
   char* cmdNoArgs[2];
   char tmpFile[MAX];

   /* Determine if filename is a valid cmd */
   if(isValidCmd(cmd) == FALSE)
   {
      return -1;
   }

   cmdNoArgs[0] = cmd;
   cmdNoArgs[1] = NULL;  

   pid = fork();

   if(pid < 0) /* Fork failed */
   {
      return -1;
   }
   else if(pid == 0) /* Child Process*/
   {  
      
      if(snprintf(tmpFile, MAX, ".%d", getpid()) < 0)
      {
         return -1;
      }

      /* Create a new file, with permissions rw */
      if((tmpFD = open(tmpFile, O_CREAT | O_RDWR, 0666)) < 0)
      {
         return -1;
      }
         
      if(dup2(tmpFD, STDOUT_FILENO) < 0)
      {
         close(tmpFD);
         return -1;
      }

      if((execvp(cmdNoArgs[0], cmdNoArgs) < 0))
      {
         perror("execl");
         close(tmpFD);
         return -1;
      }
   }
   else /* Parent Process */
   {
      if(snprintf(tmpFile, MAX, ".%d", pid) < 0)
      {
         return -1;
      }
 
      *nameOfNewTmpFile = (char*) malloc(strlen(tmpFile) + 1);
      strncpy(*nameOfNewTmpFile, tmpFile, strlen(tmpFile) + 1);

      wait(NULL);
   }

   return 1;
}

int execCgiWithArgs(char* filename, char** nameOfNewTmpFile)
{
   char** execArgs;
   char tmpFile[MAX];
   int numArgs;
   int tmpFD;
   char* arg;
   char* cmd;
   int i;
   pid_t pid;

   /* Count the number plus the number of arguments plus null terminator */
   numArgs = getNumArgs(filename);

   execArgs = (char**) malloc( sizeof(char**) * (numArgs+2));   /* 2 is, one for the cmd, and one for the command line null terminator */
   cmd = strtok(filename, "?");     

   /* Validate if cmd is valid */
   if(isValidCmd(cmd) == FALSE)
   {
      free(execArgs);
      return -1;
   }

   execArgs[0] = cmd;
   for(i = 1; i < numArgs+1; i++)
   {
      arg = strtok(NULL, "&");
      execArgs[i] = arg;
   }

   execArgs[i] = NULL;

   pid = fork();

   if(pid < 0) /* Fork failed */
   {
      free(execArgs);
      return -1;
   }
   else if(pid == 0) /* Child Process*/
   {
      if(snprintf(tmpFile, MAX, ".%d", getpid()) < 0)
      {
         free(execArgs);
         return -1;
      }
      /* Create a new file, with permissions rw */
      if((tmpFD = open(tmpFile, O_CREAT | O_RDWR, 0666)) < 0)
      {
         free(execArgs);
         return -1;
      }
            
      if(dup2(tmpFD, STDOUT_FILENO) < 0)
      {
         free(execArgs);
         close(tmpFD);
         return -1;
      }

      if((execvp(execArgs[0], execArgs)) < 0)
      {
         free(execArgs);
         perror("execvp");
         close(tmpFD);
         return -1;
      }
   }
   else /* Parent Process */
   {
      free(execArgs);

      if(snprintf(tmpFile, MAX, ".%d", pid) < 0)
      {
         return -1;
      }

      *nameOfNewTmpFile = (char*) malloc(strlen(tmpFile) + 1);
      strncpy(*nameOfNewTmpFile, tmpFile, strlen(tmpFile) + 1);

      wait(NULL);
   }

   return 1;
}

int cgiContainsArgs(char* filename)
{
   int length = strlen(filename);
   int i;

   for(i = 0; i < length; i++)
   {
      if((filename[i] == '?') || (filename[i] == '&'))
         return TRUE;
   }

   return FALSE;
}

int isCgiLike(char* filename)
{
   int length = strlen(filename);

   /* If cgi-like is present, it will be in the form of: [ ./cgi-like/... ]*/
   /* ./cgi-like/ is 10 characters, check if filename argument is less than that, then it would mean cgi-like was not requested */
   if(length <= 11)
      return FALSE;
   /* If the beginning of filename is not "/cgi-like/, then return false */
   else if(strncmp(filename, "./cgi-like/", 11) == 0)
      return TRUE;
   else
      return FALSE;
}
