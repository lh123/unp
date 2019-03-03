#include "unp.h"

static void recvfrom_alarm(int signo);
static int pipefd[2];

void dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen) {
    const int on = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }
    if (pipe(pipefd) < 0) {
        err_sys("pipe error");
    }
    int maxfdp1 = (sockfd > pipefd[0] ? sockfd : pipefd[0]) + 1;
    fd_set rset;
    FD_ZERO(&rset);

    struct sigaction act;
    act.sa_handler = recvfrom_alarm;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGALRM, &act, NULL) < 0) {
        err_sys("sigaction error");
    }

    char sendline[MAXLINE], recvline[MAXLINE + 1];
    struct sockaddr_storage reply_addr;
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen) < 0) {
            err_sys("sendto error");
        }
        alarm(5);
        for (;;) {
            FD_SET(sockfd, &rset);
            FD_SET(pipefd[0], &rset);
            int n = select(maxfdp1, &rset, NULL, NULL, NULL);
            printf("select return %d\n", n);
            if (n < 0) {
                if (errno == EINTR) {
                    printf("select EINTR\n");
                    continue;
                } else {
                    err_sys("select error");
                }
            }
            if (FD_ISSET(sockfd, &rset)) {
                socklen_t len = servlen;
                n = recvfrom(sockfd, recvline, MAXLINE, 0, (SA *)&reply_addr, &len);
                if (n < 0) {
                    err_sys("recvfrom error");
                }
                recvline[n] = 0;
                printf("from %s: %s\n", sock_ntop((SA *)&reply_addr, len), recvline);
            }
            if (FD_ISSET(pipefd[0], &rset)) {
                if (read(pipefd[0], &n, 1) < 0) {
                    err_sys("read error");
                }
                break;
            }
        }
    }
}

static void recvfrom_alarm(int signo) {
    printf("write one byte to pipe\n");
    if (write(pipefd[1], "", 1) < 0) {
        err_sys("write error");
    }
}
