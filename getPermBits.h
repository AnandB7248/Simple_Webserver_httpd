#ifndef GETPERMBITS_H
#define GETPERMBITS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

/* Parameters: char* is the filename to obtain the permission bits */
/*             char** is the place to save the permission bits */
void getPermissionBits(const char*, char**);

/* Below are helper methods to getAccessBits*/
int getFileType(char**, struct stat* );
char setExecuteBit(int, int);
void getPermission(char*, int, struct stat* );

#endif
