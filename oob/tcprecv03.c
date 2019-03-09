#include "unp.h"

int main(int argc, char **argv) {
    int listenfd;
    if (argc == 2) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else if (argc == 3) {
        listenfd = tcp_listen(argv[1], argv[2]);
    } else {
        err_quit("usage: tcprecv02 [ <host> ] <port>");
    }

    int connfd = accept(listenfd, NULL, NULL);

    fd_set rset, xset;
    FD_ZERO(&rset);
    FD_ZERO(&xset);
    char buff[100];
    int justreadoob = 0;
    for (;;) {
        FD_SET(connfd, &rset);
        if (justreadoob == 0) {
            FD_SET(connfd, &xset);
        }

        int n = select(connfd + 1, &rset, NULL, &xset, NULL);
        if (n < 0) {
            if (errno = EINTR) {
                continue;
            } else {
                err_sys("select error");
            }
        }
        if (FD_ISSET(connfd, &xset)) {
            ssize_t nr = recv(connfd, buff, sizeof(buff) - 1, MSG_OOB);
            if (nr < 0) {
                err_sys("recv error");
            }
            buff[nr] = 0;
            printf("read %d OOB byte: %s\n", (int)nr, buff);
            justreadoob = 1;
        }
        if (FD_ISSET(connfd, &rset)) {
            ssize_t nr = read(connfd, buff, sizeof(buff) - 1);
            if (nr < 0) {
                err_sys("read error");
            } else if (nr == 0) {
                printf("received EOF\n");
                exit(0);
            }
            buff[nr] = 0;
            printf("read %d bytes: %s\n", (int)nr, buff);
            justreadoob = 0;
        }
    }
}
