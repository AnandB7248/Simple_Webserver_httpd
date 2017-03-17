#ifndef CGI_UTIL_H
#define CGI_UTIL_H

int execCgiCmd(char*, char**);

int execCgiWithArgs(char*, char**);

int execCgiNoArgs(char*, char**);

/* Boolean function, checks to see /cgi-like/(contents) */
/* if contents has "?", if no ?, then no arguments. */
int cgiContainsArgs(char* contents);

/* Boolean function */
/* Checks if filename has cgi-like request */
/* e.g. [/cgi-like/... ]*/
int isCgiLike(char* filename);

#endif
