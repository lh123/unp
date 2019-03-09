#include "unp.h"
#include <fcntl.h>

void sig_urg(int signo);

static int listenfd, connfd;

ssize_t Read(int fd, char *buff, size_t nbytes);

int main(int argc, char **argv) {
    if (argc == 2) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else if (argc == 3) {
        listenfd = tcp_listen(argv[1], argv[2]);
    } else {
        err_quit("usage: tcprecv01 [ <host> ] <port>");
    }
    if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
        err_sys("accept error");
    }

    struct sigaction act;
    act.sa_handler = sig_urg;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGURG, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    if(fcntl(connfd, F_SETOWN, getpid()) < 0) {
        err_sys("fcntl error");
    }

    char buff[100];
    for (;;) {
        ssize_t n = Read(connfd, buff, sizeof(buff) - 1);
        if (n == 0) {
            printf("received EOF\n");
            exit(0);
        }
        buff[n] = 0; // null terminate
        printf("read %d bytes: %s\n", (int)n, buff);
    }
}

void sig_urg(int signo) {
    printf("SIGURG received\n");
    char buff[100];
    ssize_t n = recv(connfd, buff, sizeof(buff) - 1, MSG_OOB);
    if (n < 0) {
        err_sys("recv error");
    }
    buff[n] = 0;
    printf("read %d OOB byte: %s\n", (int)n, buff);
}

ssize_t Read(int fd, char *buff, size_t nbytes) {
    ssize_t nread;
again:
    nread = read(fd, buff, nbytes);
    if (nread < 0) {
        if (errno == EINTR) {
            goto again;
        } else {
            return nread;
        }
    }
    return nread;
}
