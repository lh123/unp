#include "unp.h"

static void sig_alrm(int signo);

void dg_cli(FILE *fp, int sockfd, const SA *servaddrptr, socklen_t servlen) {
    if (signal(SIGALRM, sig_alrm) < 0) {
        err_sys("signal error");
    }

    char sendline[MAXLINE], recvline[MAXLINE + 1];
    ssize_t n;
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, servaddrptr, servlen) < 0) {
            err_sys("sendto error");
        }
        
        alarm(5);
        if ((n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL)) < 0) {
            if (errno == EINTR) {
                fprintf(stderr, "socket timeout\n");
            } else {
                err_sys("recvfrom error");
            }
        } else {
            alarm(0);
            recvline[n] = 0;
            if (fputs(recvline, stdout) == EOF) {
                err_sys("fputs error");
            }
        }
    }
}

static void sig_alrm(int signo) {
    // do nothing
}
