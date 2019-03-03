#include "unp.h"
#include <setjmp.h>

static void recvfrom_alarm(int signo);
static sigjmp_buf jmpbuf;

void dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen) {
    const int on = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }

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
            if (sigsetjmp(jmpbuf, 1) != 0) {
                break;
            }
            socklen_t len = servlen;
            ssize_t n = recvfrom(sockfd, recvline, MAXLINE, 0, (SA *)&reply_addr, &len);
            if (n < 0) {
                err_sys("recvfrom error");
            }
            recvline[n] = 0;
            printf("from %s: %s\n", sock_ntop((SA *)&reply_addr, len), recvline);
        }
    }
    if (ferror(fp)) {
        err_sys("fgets error");
    }
}

static void recvfrom_alarm(int signo) {
    siglongjmp(jmpbuf, 1);
}
