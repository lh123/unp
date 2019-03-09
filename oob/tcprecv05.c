#include "unp.h"
#include <fcntl.h>

int listenfd, connfd;

void sig_urg(int signo);

int main(int argc, char **argv) {
    if (argc == 2) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else if (argc == 3) {
        listenfd = tcp_listen(argv[1], argv[2]);
    } else {
        err_quit("usage: tcprecv05 [ <host> ] <port>");
    }

    const int size = 4096;
    if (setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) < 0) {
        err_sys("setsockopt error");
    }

    connfd = accept(listenfd, NULL, NULL);

    struct sigaction act;
    act.sa_handler = sig_urg;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGURG, &act, NULL) < 0) {
        err_sys("sigaction error");
    }

    if (fcntl(connfd, F_SETOWN, getpid()) < 0) {
        err_sys("fcntl error");
    }

    for (;;) {
        pause();
    }
}

void sig_urg(int signo) {
    char buff[2048];
    printf("SIGURG received\n");
    ssize_t n = recv(connfd, buff, sizeof(buff) - 1, MSG_OOB);
    if (n < 0) {
        err_sys("recv error");
    }
    buff[n] = 0;
    printf("read %d OOB byte\n", (int)n);
}
