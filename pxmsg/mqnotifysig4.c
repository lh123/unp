#include "unp.h"
#include <mqueue.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: mqnotifysig4 <name>");
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
    if (buff == NULL) {
        err_sys("malloc error");
    }
    sigset_t newmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &newmask, NULL) < 0) {
        err_sys("sigprocmask error");
    }

    struct sigevent sigev;
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;
    if (mq_notify(mqd, &sigev) < 0) {
        err_sys("mq_notify error");
    }
    int signo;
    for (;;) {
        /*
         * The  sigwait()  function  suspends execution of the calling thread until one of the signals specified in the signal set set becomes pending.
         * The function accepts the signal (removes it from the pending list of signals), and returns the signal number in sig
         */
        if (sigwait(&newmask, &signo) < 0) {
            err_sys("sigwait error");
        }
        if (signo == SIGUSR1) {
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
        }
    }
}
