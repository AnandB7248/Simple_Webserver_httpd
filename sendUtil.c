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
