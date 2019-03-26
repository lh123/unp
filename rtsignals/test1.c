#include "unp.h"

static void sig_rt(int signo, siginfo_t *info, void *context);

int main(int argc, char **argv) {
    printf("SIGRTMIN = %d, SIGRTMAX = %d\n", (int)SIGRTMIN, (int)SIGRTMAX);
    pid_t pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        sigset_t newset;
        sigemptyset(&newset);
        sigaddset(&newset, SIGRTMAX);
        sigaddset(&newset, SIGRTMAX - 1);
        sigaddset(&newset, SIGRTMAX - 2);
        if (sigprocmask(SIG_BLOCK, &newset, NULL) < 0) {
            err_sys("sigprocmask error");
        }

        struct sigaction act;
        act.sa_sigaction = sig_rt;
        act.sa_flags = SA_SIGINFO;
        act.sa_mask = newset;
        if (sigaction(SIGRTMAX, &act, NULL) < 0) {
            err_sys("sigaction error");
        }
        if (sigaction(SIGRTMAX - 1, &act, NULL) < 0) {
            err_sys("sigaction error");
        }
        if (sigaction(SIGRTMAX - 2, &act, NULL) < 0) {
            err_sys("sigaction error");
        }
        sleep(6);

        if (sigprocmask(SIG_UNBLOCK, &newset, NULL) < 0) {
            err_sys("sigprocmask error");
        }
        sleep(3);
        exit(0);
    }

    sleep(3);
    int i, j;
    union sigval val;
    for (i = SIGRTMAX; i >= SIGRTMAX - 2; i--) {
        for (j = 0; j <= 2; j++) {
            val.sival_int = j;
            if (sigqueue(pid, i, val) < 0) {
                err_sys("sigqueue error");
            }
            printf("sent signal %d, val = %d\n", i, j);
        }
    }
}

static void sig_rt(int signo, siginfo_t *info, void *context) {
    printf("received signal #%d, code = %d, ival = %d\n", signo, info->si_code, info->si_value.sival_int);
}
