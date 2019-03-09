#include "unp.h"

int main(int argc, char **argv) {
    int listenfd;
    if (argc == 2) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else if (argc == 3) {
        listenfd = tcp_listen(argv[1], argv[2]);
    } else {
        err_quit("usage: tcprecv04 [ <host> ] <port>");
    }
    const int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_OOBINLINE, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }

    int connfd = accept(listenfd, NULL, NULL);
    if (connfd < 0) {
        err_sys("accept error");
    }
    sleep(5);

    char buff[100];
    for (;;) {
        if (sockatmark(connfd)) {
            printf("at OOB mark\n");
        }

        ssize_t n = read(connfd, buff, sizeof(buff) - 1);
        if (n < 0) {
            err_sys("read error");
        } else if (n == 0) {
            printf("received EOF\n");
            exit(0);
        }
        buff[n] = 0;
        printf("read %d bytes: %s\n", (int)n, buff);
    }
}
