#ifndef HANDLE_REQUEST_H
#define HANDLE_REQUEST_H

/* Main function */
void handle_request(int);

/* -------------------------------------------------- *
 * -----------------HELPER METHODS------------------- *
 * -------------------------------------------------- *
 */

/* Parses the request value from the client */
/* Function also checks for errors and returns the error codes if such an error is present */
/* Function saves the filename from the client request*/
/* Function sets typeswitch: */
/*                         1 - GET  */
/*                         2 - HEAD */
/* Function sets cgiswitch: */
/*                        0 - If no cgi-like witch given by request */
/*                        1 - If a cgi-like request is given. */
/* Return values: */
/*              Returns 1, if no errors are present   */
/*              Returns >= 400, if errors are present */
/*                400 - Bad Request */
/*                403 - Permission Denied */
/*                404 - Not Found */
/*                500 - Internal Error */
/*                501 - Not Implemented */
int parseReq(char* request, char** filename, int* typeSwitch, int* cgiSwitch);

/* If cgi-like, gets the number of arguments to cmd */
/* e.g. GET /cgi-like/ls?-l&index.html&main.html HTTP/1 */
/* There is 3 arguments to cmd ls */
/* 1. -l */
/* 2. index.html */
/* 3. main.html  */
int getNumArgs(char*);

int isValidHttp(char*);

int isValidCmd(char*);

/* Preconditions: type value is either "GET" or "HEAD" */
int retTypeSwitch(char* type);

int fileExists(char*);

char* prependDot(char* name);

/* Boolean function, true if filename starts with a slash, false otherwise */
int startsWithSlash(char*);

int fileIsDir(char* name);

int isRegFile(char*);

/* Checks if user has read permissions for file char* name */
int usrReadBitSet(char* name);

#endif
