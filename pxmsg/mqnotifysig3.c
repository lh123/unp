#include "unp.h"
#include <mqueue.h>

volatile sig_atomic_t mqflag;
static void sig_usr1(int signo);

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: mqnotifysig2 <name>");
    }
    mqd_t mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK);
    if (mqd < 0) {
        err_sys("mq_open error");
    }
    struct mq_attr attr;
    if (mq_getattr(mqd, &attr) < 0) {
        err_sys("mq_getattr error");
    }
    void *buff = malloc(attr.mq_msgsize);
    sigset_t zeromask, newmask, oldmask;
    sigemptyset(&zeromask);
    sigemptyset(&newmask);
    sigemptyset(&oldmask);
    sigaddset(&newmask, SIGUSR1);

    struct sigaction act;
    act.sa_handler = sig_usr1;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGUSR1, &act, NULL) < 0) {
        err_sys("sigaction error");
    }

    struct sigevent sigev;
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;
    if (mq_notify(mqd, &sigev) < 0) {
        err_sys("mq_notify error");
    }
    for (;;) {
        sigprocmask(SIG_BLOCK, &newmask, &oldmask);
        while (mqflag == 0) {
            sigsuspend(&zeromask);
        }
        mqflag = 0;

        if (mq_notify(mqd, &sigev) < 0) {
            err_sys("mq_notify error");
        }

        ssize_t n;
        while ((n = mq_receive(mqd, buff, attr.mq_msgsize, NULL)) >= 0) {
            printf("read %ld bytes\n", (long)n);
        } 
        if (errno != EAGAIN) {
            err_sys("mq_receive error");
        }
        sigprocmask(SIG_UNBLOCK, &newmask, NULL);
    }
}

static void sig_usr1(int signo) {
    mqflag = 1;
}
