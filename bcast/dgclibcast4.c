#include "unp.h"

static void recvfrom_alarm(int signo);

void dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen) {
    const int on = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }
    sigset_t sigset_alrm, sigset_empty;
    sigemptyset(&sigset_alrm);
    sigemptyset(&sigset_empty);
    sigaddset(&sigset_alrm, SIGALRM);

    struct sigaction act;
    act.sa_handler = recvfrom_alarm;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    fd_set rset;
    FD_ZERO(&rset);

    char sendline[MAXLINE], recvline[MAXLINE + 1];
    struct sockaddr_storage reply_addr;
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen) < 0) {
            err_sys("sendto error");
        }
        if (sigprocmask(SIG_BLOCK, &sigset_alrm, NULL) < 0) {
            err_sys("sigprocmask error");
        }
        alarm(5);
        for (;;) {
            FD_SET(sockfd, &rset);
            int n = pselect(sockfd + 1, &rset, NULL, NULL, NULL, &sigset_empty);
            if (n < 0) {
                if (errno == EINTR) {
                    break;
                } else {
                    err_sys("pselect error");
                }
            } else if (n != 1) {
                err_sys("pselect error: returned %d", n);
            }
            socklen_t len = servlen;
            if ((n = recvfrom(sockfd, recvline, MAXLINE, 0, (SA *)&reply_addr, &len)) < 0) {
                err_sys("recvfrom error");
            }
            recvline[n] = 0;
            printf("from %s: %s\n", sock_ntop((SA *)&reply_addr, len), recvline);
        }
    }
}

static void recvfrom_alarm(int signo) {}
