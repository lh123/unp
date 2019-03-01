#include "unp.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: tcpserv <hostname/IPaddress> <service/port>");
    }
    int listenfd = tcp_listen(argv[1], argv[2]);

    int maxfd = listenfd;
    int clifds[MAXFD];
    int maxi = 0;

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(listenfd, &rset);

    int i;
    for (i = 0; i < MAXFD; i++) {
        clifds[i] = -1;
    }

    char buf[MAXLINE];
    for (;;) {
        fd_set rs = rset;
        int n = select(maxfd + 1, &rs, NULL, NULL, NULL);
        if (n < 0) {
            if (errno != EINTR) {
                err_sys("select error");
            } else {
                continue;
            }
        }
        if (FD_ISSET(listenfd, &rs)) {
            printf("listening socket readable\n");
            sleep(5);
            int connfd = accept(listenfd, NULL, NULL);
            if (connfd < 0) {
                err_sys("accept error");
            }
            for (i = 0; i < MAXFD; i++) {
                if (clifds[i] == -1) {
                    clifds[i] = connfd;
                    break;
                }
            }
            if (i == MAXFD) {
                printf("too many client\n");
                close(connfd);
            } else {
                if (maxfd < connfd) {
                    maxfd = connfd;
                }
                if (maxi < i) {
                    maxi = i;
                }
                FD_SET(connfd, &rset);
            }
            if (--n <= 0) {
                continue;
            }
        }
        for (i = 0; i <= maxi; i++) {
            int sockfd = clifds[i];
            if (sockfd == -1) {
                continue;
            }
            if (FD_ISSET(sockfd, &rs)) {
                int nread = read(sockfd, buf, MAXLINE);
                if (nread < 0) {
                    if (errno == ECONNRESET) {
                        close(sockfd);
                        FD_CLR(sockfd, &rset);
                        clifds[i] = -1;
                        printf("client %d send rst\n", sockfd);
                    } else {
                        err_sys("read error");
                    }
                } else if (nread == 0) {
                    // eof
                    close(sockfd);
                    FD_CLR(sockfd, &rset);
                    clifds[i] = -1;
                    printf("client %d exit\n", sockfd);
                } else {
                    if (writen(sockfd, buf, nread) < 0) {
                        err_sys("writen error");
                    }
                }
                if (--n <= 0) {
                    break;
                }
            }
        }
    }
}
