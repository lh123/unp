#include "unp.h"
#include <mqueue.h>

int pipefd[2];
static void sig_usr1(int signo);

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: mqnotifysig5 <name>");
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
    if (pipe(pipefd) < 0) {
        err_sys("pipe error");
    }

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

    fd_set rset;
    FD_ZERO(&rset);
    for (;;) {
        FD_SET(pipefd[0], &rset);
        int nfds = select(pipefd[0] + 1, &rset, NULL, NULL, NULL);
        if (nfds < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                err_sys("select error");
            }
        }
        if (FD_ISSET(pipefd[0], &rset)) {
            char c;
            if (read(pipefd[0], &c, 1) < 0) {
                err_sys("read error");
            }
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

static void sig_usr1(int signo) {
    // write 异步信号安全
    if (write(pipefd[1], "", 1) < 0) {
        err_sys("write error");
    }
}
