#ifndef SEND_UTIL_H
#define SEND_UTIL_H

void sendError(int errorCode, int socketFD);

void sendHead(char* filename, int socketFD);

void sendGet(char* filename, int socketFD);

#endif
