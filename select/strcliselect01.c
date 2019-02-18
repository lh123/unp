#include "unp.h"
#include <sys/select.h>

void str_cli(FILE *fp, int sockfd) {
    int maxfd;
    fd_set rset;
    char sendline[MAXLINE], recvline[MAXLINE];
    FD_ZERO(&rset);

    for (;;) {
        FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfd = (fileno(fp) > sockfd ? fileno(fp) : sockfd) + 1;
        if (select(maxfd, &rset, NULL, NULL, NULL) < 0) {
            err_sys("select error");
        }
        if (FD_ISSET(sockfd, &rset)) {
            int nr;
            if ((nr = readline(sockfd, recvline, MAXLINE)) < 0) {
                err_sys("readline error");
            } else if (nr == 0) {
                err_quit("str_cli: server terminated prematurely");
            }
            if (fputs(recvline, stdout) == EOF) {
                err_sys("fputs error");
            }
        }
        if (FD_ISSET(fileno(fp), &rset)) {
            if (fgets(sendline, MAXLINE, fp) == NULL) {
                return;
            }
            if (writen(sockfd, sendline, strlen(sendline)) < 0) {
                err_sys("writen error");
            }
        }
    }
}
