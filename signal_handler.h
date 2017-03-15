#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>

void signal_setup(int signo);
void signal_handler(int signo);

#endif
