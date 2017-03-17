#ifndef HANDLE_REQUEST_H
#define HANDLE_REQUEST_H

void handle_request(int);

/* -------------------------------------------------- *
 * -----------------HELPER METHODS------------------- *
 * -------------------------------------------------- *
 */

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

int getNumArgs(char*);

int isValidHttp(char*);

int isValidCmd(char*);

/* Preconditions: type value is either "GET" or "HEAD" */
int retTypeSwitch(char* type);

int fileExists(char*);

char* prependDot(char* name);

int startsWithSlash(char*);

int fileIsDir(char* name);

int isRegFile(char*);

int usrReadBitSet(char* name);

#endif
