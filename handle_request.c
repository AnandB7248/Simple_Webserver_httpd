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

void handle_request(int socket)
{
   char* request;
   char* filename;
   char* tmpFile;
   int status;
   int typeSwitch;
   int cgiSwitch;
   FILE* client = fdopen(socket, "r");

   /* If fdopen did not work */
   if(client == NULL)
   {
      sendError(INTERNAL_ERROR, socket);
      fclose(client);
      close(socket);
      exit(-1);
   }

   /* Receive */
   request = readline(client);

   if((status = parseReq(request, &filename, &typeSwitch, &cgiSwitch)) >= ERROR_LIMIT)
   {
      free(request);
      /* Send Error Response */
      sendError(status, socket);
      fclose(client);
      close(socket);
      exit(-1);
   }
   else 
   {
      /* If cgi-like switch given */
      if(cgiSwitch == TRUE)
      {
         printf("cgi-like\n");

         if(execCgiCmd(filename, &tmpFile) < 0)
         {
            free(request);
            sendError(INTERNAL_ERROR, socket);
            fclose(client);
            close(socket);
            exit(-1);
         }
         else
         {
            printf("IN MAIN, Name of tmpFile: %s\n", tmpFile);
         
            /* If head */
            if(typeSwitch == HEAD_SWITCH)
            {
               sendHead(tmpFile, socket);
            }
            /* If get */
            else if(typeSwitch == GET_SWITCH)
            {
               sendHead(tmpFile, socket);
               sendGet(tmpFile, socket);
            }
         
            /* Remove the tmp file */
            if(remove(tmpFile) < 0)
            {
               free(tmpFile);
               sendError(INTERNAL_ERROR, socket);
               fclose(client);
               close(socket);
               exit(-1);
            }
         }
         free(tmpFile);
      }

      /* If cgi-like switch not given */
         /* We know file exists with readbit set, due parseReq checking for these informations. */
      else if(cgiSwitch == FALSE)
      {
         printf("not cgi-like\n");
         if(typeSwitch == HEAD_SWITCH)
         {
            sendHead(filename, socket);
         }
         else if(typeSwitch == GET_SWITCH)
         {
            sendHead(filename, socket);
            sendGet(filename, socket);
         }
      }
   }
   
   /* Clean up resources */
   free(request);
   free(filename);
   fclose(client);
   close(socket);
   exit(0);
}

int parseReq(char* request, char** filename, int* typeS, int* cgiS)
{
   char* part; 
   char* httpV;
   char* type;
   int numItems = 0;

   part = strtok(request, " ");
   while(part)
   {
      if(numItems == 0)
         type = part;
      if(numItems == 1)
         *filename = part;
      if(numItems == 2)
         httpV = part;

      part = strtok(NULL, " ");
      numItems++;
   }

   if(numItems != 3)
   {  /* Request has the form: TYPE filename HTTP/version */
      /* Which has 3 parts, so if numItems is not 3, return the error code */
      return BAD_REQUEST; /* Error 400*/
   }

   /* Modify filename so it starts with a "." */
   *filename = convNameToCurDir(*filename);

   /* If TYPE is not GET or HEAD, send Error 501 */
   if((strcmp(type, "GET") != 0) && (strcmp(type, "HEAD") != 0))
   {
      free(*filename);
      return NOT_IMPLEMENTED;
   }
   /* --- filename --- */
   /* Check if filename is trying to access directory: ".." */
   /* SHOULD MAYBE ALSO CHECK for "~/" ??? */
   if((strstr(*filename, "..") != NULL) || (strstr(*filename, "~/") != NULL))
   {
      free(*filename);
      return BAD_REQUEST; /* Not sure what error to send here */
   }

   /* If cgi is requested */
   if(cgiRequested(*filename))
   {
      *cgiS = TRUE;
   }
   else
   {  /* No cgi-request, should be a file or directory*/
      *cgiS = FALSE;

      /* Check if file exists */
      if(!fileExists(*filename)) /* TEST */
      {
         free(*filename);
         return NOT_FOUND;
      }
      /* Check if filename leads to just a directory */
      if(fileIsDir(*filename))
      {
         printf("Directory\n");
         free(*filename);
         return BAD_REQUEST; /* Not sure what error to send here */
      }
      
      /* Check if there is a read permission for file*/
      if(!usrReadBitSet(*filename))
      {
         free(*filename);
         return PERMISSION_DENIED; 
      }
   }

   /* http */
   /* If request line does not contain "HTTP/", send error 400*/
   if(!isValidHttp(httpV))
   {
      free(*filename);
      return BAD_REQUEST; /* Error 400*/
   }

   /* Set typeswitch */ 
   *typeS = retTypeSwitch(type); /* QUESTION: Would there be a reason where we would need to somehow safe and later reference http/version??? */

   return 1;
}

int execCgiCmd(char* filename, char** nameOfNewTmpFile)
{
   /* By the way this program is written, filename argument is guaranteed to be: ./cgi-like/(contents) */
   char* contents = NULL;
   char* filenameCopy;
   char* tmp;
   char* arg;
   char* cmd;
   char tmpFile[MAX];
   char** execArgs;
   char*  cmdNoArgs[2];
   int tmpFD;
   int numArgs;
   pid_t pid;
   char backSlash[2] = "/";
   char questionM[2] = "?";
   char ampersand[2] = "&";
   int filenameLength = strlen(filename);
   int i;

   filenameCopy = (char*) malloc(filenameLength + 1);

   printf("Original Filename: %s\n", filename);
   strncpy(filenameCopy, filename, strlen(filename));
   filenameCopy[filenameLength] = '\0';

   if(filenameCopy == NULL)
   {
      return -1;
   }

   printf("Copied Filename: %s\n", filenameCopy);

   /* Let's remove "./cgi-like/" from filename, and have char* contents be string value after /cgi-like/ */
   tmp = strtok(filenameCopy, backSlash);
   tmp = strtok(NULL, backSlash);
   contents = strtok(NULL,backSlash);

   printf("Contents: [%s]\n", contents);
   /* If cmd has arguments */
   if(cgiContainsArgs(contents))
   {
      printf("Contains arguments\n");
      /* Count the number plus the number of arguments plus null terminator */
      numArgs = getNumArgs(contents);
      printf("Number of arguments: %d\n", numArgs);
      execArgs = (char**) malloc(sizeof(char**)*(numArgs + 2));   /* 2 is, one for the cmd, and one for the command line null terminator */
      arg = strtok(contents, questionM);
      printf("cmd: [%s]\n", arg);
      /* TEST */
      execArgs[0] = arg;
      printf("Should be cmd: %s\n", execArgs[0]);    

      for(i = 1; i < numArgs+1; i++)
      {
         arg = strtok(NULL, ampersand);
         execArgs[i] = arg;
         printf("Argument:%s\n", execArgs[i]);
      }
      execArgs[i] = NULL;

      i = 0;
      while(execArgs[i] != NULL)
      {
         printf("ARGUMENT: %s\n", execArgs[i++]);
      }

      printf("Original filename: %s\n", filename);
      cmd = strtok(filename, questionM);
      printf("Path to execute is: %s\n", cmd);

      pid = fork();
      if(pid < 0) /* Fork failed */
      {
         return -1;
      }
      else if(pid == 0) /* Child Process*/
      {
         printf("PID before: %d\n", getpid());
         if(snprintf(tmpFile, MAX, "%d", getpid()) < 0)
         {
            return -1;
         }
         printf("PID after: %s\n", tmpFile);
         /* Create a new file, with permissions rw */
         if((tmpFD = open(tmpFile, O_CREAT | O_RDWR, 0666)) < 0)
         {
            return -1;
         }
            
         if(dup2(tmpFD, STDOUT_FILENO) < 0)
         {
            printf("dup2\n");
            close(tmpFD);
            return -1;
         }

         chdir("./cgi-like/");

         if((execvp(execArgs[0], execArgs)) < 0)
         {
            printf("EXECVP failed\n");
            perror("execvp");
            close(tmpFD);
            return -1;
         }
      }
      else /* Parent Process */
      {
         if(snprintf(tmpFile, MAX, "%d", pid) < 0)
         {
            return -1;
         }
         printf("Parent Process number : %d\n", pid);
         printf("Parent Process string : %s\n", tmpFile);

         *nameOfNewTmpFile = (char*) malloc(strlen(tmpFile) + 1);
         strncpy(*nameOfNewTmpFile, tmpFile, strlen(tmpFile));
         printf("TEST, should be same as Parent: %s\n", *nameOfNewTmpFile);

         wait(NULL);

         return 1;
      }
   }
   /* If cmd has no arguments */
   else
   {
      /* No longer need filenameCopy */
      free(filenameCopy);

      printf("Does not contain arguments\n");
      /* This would then mean all of char* contents would be a potential cmd */
      printf("cmd: [%s]\n", contents);
      pid = fork();

      if(pid < 0) /* Fork failed */
      {
         return -1;
      }
      else if(pid == 0) /* Child Process*/
      {
         printf("PID before: %d\n", getpid());
         if(snprintf(tmpFile, MAX, "%d", getpid()) < 0)
         {
            return -1;
         }
         printf("PID after: %s\n", tmpFile);
         /* Create a new file, with permissions rw */
         if((tmpFD = open(tmpFile, O_CREAT | O_RDWR, 0666)) < 0)
         {
            return -1;
         }
         printf("filename is: %s\n", filename);         
         
         
         if(dup2(tmpFD, STDOUT_FILENO) < 0)
         {
            printf("dup2\n");
            close(tmpFD);
            return -1;
         }

         chdir("./cgi-like/");
         cmdNoArgs[0] = contents;
         cmdNoArgs[1] = NULL;
         if((execvp(cmdNoArgs[0], cmdNoArgs) < 0))
         {
            printf("EXECL failed\n");
            perror("execl");
            close(tmpFD);
            return -1;
         }
      }
      else /* Parent Process */
      {
         if(snprintf(tmpFile, MAX, "%d", pid) < 0)
         {
            return -1;
         }
         printf("Parent Process number : %d\n", pid);
         printf("Parent Process string : %s\n", tmpFile);

         *nameOfNewTmpFile = (char*) malloc(strlen(tmpFile) + 1);
         strncpy(*nameOfNewTmpFile, tmpFile, strlen(tmpFile));
         printf("TEST, should be same as Parent: %s\n", *nameOfNewTmpFile);

         wait(NULL);
      }
      /* Remember to free filenameCopy */
   }

   return 1;
}

int cgiContainsArgs(char* contents)
{
   int length = strlen(contents);
   int i;

   for(i = 0; i < length; i++)
   {
      if(contents[i] == '?')
      {
         return TRUE;
      }
   }

   return FALSE;
}

/* char* contents guaranteed to be : cmd? */
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
   char* errorResponseLeng = "Content-Length: 0\r\n\0";

   if(errorCode == BAD_REQUEST)
      errorResponse = "HTTP/1.0 400 Bad Request\r\n\0";
   else if(errorCode == PERMISSION_DENIED)
      errorResponse = "HTTP/1.0 403 PermissionDenied\r\n\0";
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
   char* command;

   /* dup stdout to socketFD */
   if(dup2(socketFD, STDOUT_FILENO) < 0)
   {
      sendError(INTERNAL_ERROR, socketFD);
   }

   command = dynamicStrCat("cat ", filename);

   /* Do a system call, calling cat filename */
   system(command);  /* QUESTION: Is it okay to use system cal for sendGet??? */
   
   free(command);
}

int isValidHttp(char* httpV)
{
   int length = strlen(httpV);
   /* HTTP/ , that is a minimum of 5 characters needed for a valid HTTP/version */
   printf("%s\n", httpV);
   printf("%d\n", length);
   if(length <= 6)
   {
      return FALSE;
   }
   else if(strncmp(httpV, "HTTP/", 5))
      return FALSE;
   else
      return TRUE;
}

int cgiRequested(char* filename)
{
   int length = strlen(filename);

   /* If cgi-like is present, it will be in the form of: [ /cgi-like/... ]*/
   /* /cgi-like/ is 10 characters, check if filename argument is less than that, then it would mean cgi-like was not requested */
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

char* convNameToCurDir(char* name)
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
