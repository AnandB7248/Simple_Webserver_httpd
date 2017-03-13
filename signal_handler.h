#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>

void sigchld_setup(int signo);
void handle_sigchld(int signo);

#endif
