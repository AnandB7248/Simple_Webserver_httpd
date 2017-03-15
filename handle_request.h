#ifndef HANDLE_REQUEST_H
#define HANDLE_REQUEST_H

void handle_request(int);

/* Parses the request value from the client */
/* Function also checks for errors and returns the error codes if such an error is present */
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

int execCgiCmd(char*, char**);

/* Boolean function, checks to see /cgi-like/(contents) */
/* if contents has "?", if no ?, then no arguments. */
int cgiContainsArgs(char* contents);

int getNumArgs(char*);

void sendError(int errorCode, int socketFD);

void sendHead(char* filename, int socketFD);

void sendGet(char* filename, int socketFD);

int isValidHttp(char*);

/* Boolean function */
/* Checks if filename has cgi-like request */
/* e.g. [/cgi-like/... ]*/
int cgiRequested(char* filename);

/* Preconditions: type value is either "GET" or "HEAD" */
int retTypeSwitch(char* type);

int fileExists(char*);

/* Modifies the name that is passed in to have it start with "./" */
char* convNameToCurDir(char* name);

int fileIsDir(char* name);

int usrReadBitSet(char* name);
#endif