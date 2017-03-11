#ifndef SIMPLE_NET_H
#define SIMPLE_NET_H

#include <sys/socket.h>

int create_service(unsigned short port, int queue_size);
int accept_connection(int fd);

#endif
