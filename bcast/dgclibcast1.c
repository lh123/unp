#include "unp.h"

static void recvfrom_alarm(int signo);

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
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen) < 0) {
            err_sys("sendto error");
        }
        alarm(5); // 信号可能在 for 中的任何位置递达，如果不是在 recvfrom 阻塞是递达那么将会导致下一次 recvfrom 永远阻塞
        for (;;) {
            socklen_t len = servlen;
            struct sockaddr_storage reply_addr;
            ssize_t n = recvfrom(sockfd, recvline, MAXLINE, 0, (SA *)&reply_addr, &len);
            if (n < 0) {
                if (errno == EINTR) {
                    break;
                } else {
                    err_sys("recvfrom error");
                }
            } else {
                recvline[n] = 0;
                printf("from %s: %s", sock_ntop((SA *)&reply_addr, len), recvline);
            }
        }
    }
}

static void recvfrom_alarm(int signo) {
    
}
