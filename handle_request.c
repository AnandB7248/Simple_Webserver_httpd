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
#include "cgiUtil.h"
#include "readline.h"
#include "getPermBits.h"
#include "sendUtil.h"
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

   /* Receive a request from the client */
   request = readline(client); /* NOTE: request should be freed at some point!*/

   /* parseReq determines if the request is valid by convention. */
   /* If invalid request, proper status error code will be returned */
   /* Which will lead to send of error back to client*/
   if((status = parseReq(request, &filename, &typeSwitch, &cgiSwitch)) >= ERROR_LIMIT)
   {
      free(request);
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
         if(execCgiCmd(filename, &tmpFile) < 0)
         {
            free(filename);
            free(request);
            sendError(INTERNAL_ERROR, socket);
            fclose(client);
            close(socket);
            exit(-1);
         }
         else
         {
            if(typeSwitch == HEAD_SWITCH)
               sendHead(tmpFile, socket); 
            else if(typeSwitch == GET_SWITCH)
               sendGet(tmpFile, socket);  

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
         /* We know file exists with readbit set, due to parseReq checking for these informations. */
      else
      {
         if(typeSwitch == HEAD_SWITCH)
            sendHead(filename, socket);
         else if(typeSwitch == GET_SWITCH)
            sendGet(filename, socket);
      }
   }
   
   /* Clean up resources */
   free(request);
   free(filename);
   fclose(client);
   close(socket);
   exit(0);
}

/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------HELPER METHODS---------------------------------------- */
/* ------------------------------------------------------------------------------------------------ */

int parseReq(char* request, char** filename, int* typeS, int* cgiS)
{
   char* partOfRequest;
   char* httpV;
   char* type;
   int numItems = 0;

   /* Request expected proper usage: TYPE filename HTTP/verson */
   partOfRequest = strtok(request, " ");
   while(partOfRequest)
   {
      if(numItems == 0)
         type = partOfRequest;
      if(numItems == 1)
         *filename = partOfRequest; /* *filename value will be stored and will be used in other functions */
      if(numItems == 2)
         httpV = partOfRequest;

      partOfRequest = strtok(NULL, " ");
      numItems++;
   }

   /* Proper request format: TYPE filename HTTP/version */
   /* So a proper request format should have 3 items */
   if(numItems < 3)
      return BAD_REQUEST; /* Error 400 */

   /* If TYPE is not GET or HEAD, send Error 501 */
   if((strcmp(type, "GET") != 0) && (strcmp(type, "HEAD") != 0))
      return NOT_IMPLEMENTED; /* Error 501 */

   /* Modify filename so it starts with a "." */
   /* This will be very helpful to find filename as search starts from the working directory of the server */
   /* And the client will most likely not place "." in front of the filename */
   *filename = prependDot(*filename);

   /* Check if filename is trying to access directory: ".." or "~/", as we want to limit the areas where the client can access */
   if((strstr(*filename, "..") != NULL) || (strstr(*filename, "~/") != NULL))
   {
      free(*filename);
      return BAD_REQUEST;
   }

   /* If cgi is requested */
   if(isCgiLike(*filename))
   {
      if(cgiContainsArgs(*filename))
         *cgiS = TRUE;
      else
      {
         if(!fileExists(*filename))
         {
            free(*filename);
            return NOT_FOUND;
         }
         
         if(isRegFile(*filename))
            *cgiS = FALSE;
         else
            *cgiS = TRUE;
      }
   }
   else
   {  /* No cgi-request, so requested filename should just be a file or directory*/
      *cgiS = FALSE;

      /* Check if file exists */
      if(!fileExists(*filename))
      {
         free(*filename);
         return NOT_FOUND; /* Error 404 */
      }

      /* Check if filename leads to just a directory */
      if(fileIsDir(*filename))
      {
         free(*filename);
         return BAD_REQUEST; /* Error 400 */
      }
         
      /* Check if there is a read permission for file */
      if(!usrReadBitSet(*filename))
      {
         free(*filename);
         return PERMISSION_DENIED; /* Error 403 */
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
   *typeS = retTypeSwitch(type);

   return 1;
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

int isRegFile(char* name)
{
   char* permissionBits;

   getPermissionBits(name, &permissionBits);

   if((permissionBits[0] == '-') && (permissionBits[1] == 'r') && (permissionBits[3] == '-'))
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
