#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "signalfd.h"

int sig_pipe[2];

void write_signal(int signal) {
    if (write(sig_pipe[1], &signal, sizeof(int)) == -1) {
        exit(EXIT_FAILURE);
    }
}

int signalfd() {
    if (pipe(sig_pipe) == -1) {
        exit(EXIT_FAILURE);
    }
    
    struct sigaction sa;
    sa.sa_handler = write_signal;
    sa.sa_flags = 0;
    
    sigemptyset(&sa.sa_mask);
    
    for (int sig_num = 1; sig_num < 32; ++sig_num) {
        if ((sig_num != SIGKILL) && (sig_num != SIGSTOP)) {
            if (sigaction(sig_num , &sa, NULL) == -1) {
                exit(EXIT_FAILURE);
            }
        }
    }
    return sig_pipe[0];
}
