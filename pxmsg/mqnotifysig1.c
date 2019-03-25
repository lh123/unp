#include "unp.h"
#include <mqueue.h>

mqd_t mqd;
void *buff;
struct mq_attr attr;
struct sigevent sigev;

static void sig_usr1(int signo);

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: mqnotifysig1 <name>");
    }
    if ((mqd = mq_open(argv[1], O_RDONLY)) < 0) {
        err_sys("mq_open error");
    }
    if (mq_getattr(mqd, &attr) < 0) {
        err_sys("mq_getattr error");
    }
    buff = malloc(attr.mq_msgsize);

    struct sigaction act;
    act.sa_handler = sig_usr1;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGUSR1, &act, NULL) < 0) {
        err_sys("sigaction error");
    }

    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;
    if (mq_notify(mqd, &sigev) < 0) {
        err_sys("mq_notify error");
    }
    for (;;) {
        pause();
    }
}

static void sig_usr1(int signo) {
    if (mq_notify(mqd, &sigev) < 0) {
        err_sys("mq_notify error");
    }
    ssize_t n = mq_receive(mqd, buff, attr.mq_msgsize, NULL);
    if (n < 0) {
        err_sys("mq_receive error");
    }
    printf("SIGUSR1 received, read %ld bytes\n", (long)n);
}
