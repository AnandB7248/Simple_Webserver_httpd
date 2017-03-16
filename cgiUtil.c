#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "handle_request.h"
#include "readline.h"
#include "getPermBits.h"
#include "dynamicStrCat.h"

/* ---- Error Codes ---- */
#define ERROR_LIMIT 400
#define BAD_REQUEST 400
#define PERMISSION_DENIED 403
#define NOT_FOUND 404
#define INTERNAL_ERROR 500
#define NOT_IMPLEMENTED 501
/* --------------------- */
#define TRUE 1
#define FALSE 0
#define GET_SWITCH 1
#define HEAD_SWITCH 2
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
   execArgs = (char**) malloc(sizeof(char**)*(numArgs + 1));   /* 2 is, one for the cmd, and one for the command line null terminator */
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
      if(snprintf(tmpFile, MAX, "%d", getpid()) < 0)
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

      if(snprintf(tmpFile, MAX, "%d", pid) < 0)
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

int getNumArgs(char* contents)
{
   int numArgs = 0;
   int length = strlen(contents);
   int i;

   for(i = 0; i < length; i++)
   {
      if((contents[i] == '?') || contents[i] == '&')
         numArgs++;
   }

   return numArgs;
}

void sendError(int errorCode, int socketFD)
{
   char* errorResponse;
   char* errorResponseType = "Content-Type: text/html\r\n\0";
   char* errorResponseLeng = "Content-Length: 0\r\n\r\n\0";

   if(errorCode == BAD_REQUEST)
      errorResponse = "HTTP/1.0 400 Bad Request\r\n\0";
   else if(errorCode == PERMISSION_DENIED)
      errorResponse = "HTTP/1.0 403 Permission Denied\r\n\0";
   else if(errorCode == NOT_FOUND) 
      errorResponse = "HTTP/1.0 404 Not Found\r\n\0";
   else if(errorCode == INTERNAL_ERROR)
      errorResponse = "HTTP/1.0 500 Internal Error\r\n\0";
   else if(errorCode == NOT_IMPLEMENTED)
      errorResponse = "HTTP/1.0 501 Not Implemented\r\n\0";

   send(socketFD, errorResponse, strlen(errorResponse), 0);
   send(socketFD, errorResponseType, strlen(errorResponseType), 0);
   send(socketFD, errorResponseLeng, strlen(errorResponseLeng), 0);
}

void sendHead(char* filename, int socketFD)
{
   char buffer[MAX];
   unsigned long contentLength;
   char* headFirstPart = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: \0";
   char* headSecondPart = "\r\n\r\n";
   struct stat buf;

   if(stat(filename, &buf))
   {
      /* If stat does not work, send internal error and exit */
      sendError(INTERNAL_ERROR, socketFD);
   }

   contentLength = (long) buf.st_size;
   sprintf(buffer, "%ld", contentLength);

   send(socketFD, headFirstPart, strlen(headFirstPart), 0);
   send(socketFD, buffer, strlen(buffer), 0);
   send(socketFD, headSecondPart, strlen(headSecondPart), 0);
}

void sendGet(char* filename, int socketFD)
{
   pid_t pid;
   char buffer[MAX];
   unsigned long contentLength;
   char* headFirstPart = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: \0";
   char* headSecondPart = "\r\n\r\n";
   struct stat buf;

   if(stat(filename, &buf))
   {
      /* If stat does not work, send internal error and exit */
      sendError(INTERNAL_ERROR, socketFD);
   }

   contentLength = (long) buf.st_size;
   sprintf(buffer, "%ld", contentLength);

   send(socketFD, headFirstPart, strlen(headFirstPart), 0);
   send(socketFD, buffer, strlen(buffer), 0);
   send(socketFD, headSecondPart, strlen(headSecondPart), 0);

   /* dup stdout to socketFD */
   if(dup2(socketFD, STDOUT_FILENO) < 0)
   {
      sendError(INTERNAL_ERROR, socketFD);
   }

   pid = fork();
   if(pid < 0)
   {
      sendError(INTERNAL_ERROR, socketFD);
   }
   else if(pid == 0)
   {
      if(execl("/bin/cat", "cat", filename, (char*) NULL) < 0)
      {
         perror("execlp");
         exit(-1);
      }
   }
   else
   {
      wait(NULL);
   }
   /* command = dynamicStrCat("cat ", filename);

   system(command);  
   
   free(command);*/
}

int isValidHttp(char* httpV)
{
   int length = strlen(httpV);
   /* HTTP/ , that is a minimum of 5 characters needed for a valid HTTP/version */
   if(length <= 6)
   {
      return FALSE;
   }
   else if(strncmp(httpV, "HTTP/", 5))
      return FALSE;
   else
      return TRUE;
}

int isValidCmd(char* filename)
{
   /* Check if the file with associated filename exists and check to see if the exec bit is set */
   char* permissionBits;

   if(fileExists(filename) == FALSE)
   {
      return FALSE;
   }

   getPermissionBits(filename, &permissionBits);
   
   if(permissionBits[3] == 'x')
   {
      free(permissionBits);
      return TRUE;
   }
   else
   {
      free(permissionBits);
      return FALSE;
   }
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

/* Preconditions to the function: *type is either "GET" or "HEAD" */
int retTypeSwitch(char* type)
{
   if(strcmp(type, "GET") == 0)
      return GET_SWITCH;
   else if(strcmp(type, "HEAD") == 0)
      return HEAD_SWITCH;
   else 
      return FALSE;
}

int fileExists(char* name)
{
   if(access(name, F_OK) != -1)
      return TRUE;
   else
      return FALSE;
}

char* prependDot(char* name)
{
   int length;
   char* newName;
   int newLength;

   if(name == NULL)
   {
      return NULL;
   }

   length = strlen(name);
   /* Malloc size of: length = current name */
   /*                      1 = for "."     */
   /*                      1 = for "\0"     */
   newLength = length + 1 + 1;
   newName = (char*) malloc(newLength);

   newName[0] = '.';
   strncat(newName, name, length);

   newName[newLength - 1] = '\0'; 

   return newName;
}

int fileIsDir(char* name)
{
   char* permissionBits;

   getPermissionBits(name, &permissionBits);

   if(permissionBits[0] == 'd')
   {
      free(permissionBits);
      return TRUE;
   }
   else
   {
      free(permissionBits);
      return FALSE;
   }
}

int usrReadBitSet(char* name)
{
   char* permissionBits;

   getPermissionBits(name, &permissionBits);

   if(permissionBits[1] == 'r')
   {
      free(permissionBits);
      return TRUE;
   }
   else
   {
      free(permissionBits);
      return FALSE;
   }
}
