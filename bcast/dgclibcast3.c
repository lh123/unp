#include "unp.h"

static void recvfrom_alarm(int signo);

static volatile sig_atomic_t had_alarm = 0;

void dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen) {
    const int on = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }
    sigset_t sigset_alrm;
    sigemptyset(&sigset_alrm);
    sigaddset(SIGALRM, &sigset_alrm);

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
            socklen_t len = servlen;
            if (sigprocmask(SIG_UNBLOCK, &sigset_alrm, NULL) < 0) {
                err_sys("sigprocmask error");
            }
            if (had_alarm == 1) {
                break;
            }
            ssize_t n = recvfrom(sockfd, recvline, MAXLINE, 0, (SA *)&reply_addr, &servlen);
            // 信号在发生在这里，将会导致信号丢失，下次的 recvfrom 将会一直阻塞.
            if (sigprocmask(SIG_BLOCK, &sigset_alrm, NULL) < 0) {
                err_sys("sigprocmask error");
            }
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
    had_alarm = 1;
}
