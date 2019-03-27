#include "unp.h"

void sig_chld(int signo) {
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("\nchild %d exit\n", pid);
    }
    if (errno != ECHILD) {
        err_sys("waitpid error");
    }
}
