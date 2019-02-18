#include "unp.h"
#include <sys/select.h>

void str_cli(FILE *fp, int sockfd) {
    int maxfd, stdineof;
    fd_set rset;
    char buf[MAXLINE];
    int n;

    stdineof = 0;
    FD_ZERO(&rset);
    for (;;) {
        if (stdineof == 0) {
            FD_SET(fileno(fp), &rset);
        }
        FD_SET(sockfd, &rset);
        maxfd = (sockfd > fileno(fp) ? sockfd : fileno(fp)) + 1;
        if (select(maxfd, &rset, NULL, NULL, NULL) < 0) {
            err_sys("select error");
        }
        if (FD_ISSET(sockfd, &rset)) {
            if ((n = read(sockfd, buf, MAXLINE)) < 0) {
                err_sys("read error");
            } else if (n == 0) {
                if (stdineof == 1) {
                    return;
                } else {
                    err_quit("str_cli: server terminated prematurely");
                }
            }
            if (write(fileno(stdout), buf, n) < 0) {
                err_sys("write error");
            }
        }
        if (FD_ISSET(fileno(fp), &rset)) {
            if ((n = read(fileno(fp), buf, MAXLINE)) < 0) {
                err_sys("read error");
            } else if (n == 0) {
                stdineof = 1;
                if (shutdown(sockfd, SHUT_WR) < 0) {
                    err_sys("shutdown error");
                }
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            if (writen(sockfd, buf, n) < 0) {
                err_sys("writen error");
            }
        }
    }
}
