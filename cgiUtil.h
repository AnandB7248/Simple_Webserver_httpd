#ifndef CGI_UTIL_H
#define CGI_UTIL_H

/* Main function */
int execCgiCmd(char*, char**);

/* --------------------------------------------- */
/* ------------------HELPER METHODS------------- */
/* --------------------------------------------- */

/* Function is executed when clients request is a cgi-like where the cmd has arguments */
int execCgiWithArgs(char*, char**);

/* Function is executed when clients request is a cgi-like where the cmd has no arguments */
int execCgiNoArgs(char*, char**);

/* Boolean function, checks to see /cgi-like/(contents) */
/* if contents has "?", if no ?, then no arguments. */
int cgiContainsArgs(char* contents);

/* Boolean function */
/* Checks if filename has cgi-like request */
/* e.g. [/cgi-like/... ]*/
int isCgiLike(char* filename);

#endif
