#include "unp.h"

static void sig_int(int signo) {
    printf("\n%ld: SIGINT\n", (long)getpid());
}

int main(void) {
    struct sigaction act;
    act.sa_handler = sig_int;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGINT, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    pid_t pid;
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        for(;;) {
            pause();
        }
        exit(0);
    }
    for(;;) {
        pause();
    }
}
