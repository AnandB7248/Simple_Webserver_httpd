#ifndef GET_PORT_H
#define GET_PORT_H

/* Command line usage: ./http [port] */
/* port : [1024 : 65535] */
/* Returns the port value passed in from the command line */
int getPort(int argc, char** argv);

#endif
