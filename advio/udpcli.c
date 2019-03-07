#include "unp.h"

void dg_cli(int sockfd, struct sockaddr *pservaddr, socklen_t servlen);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: udpcli <hostname/IPaddress> <service/port#>");
    }
    struct sockaddr_storage servaddr;
    socklen_t servlen = sizeof(servaddr);
    int sockfd = udp_client(argv[1], argv[2], (SA *)&servaddr, &servlen);
    if (sockfd < 0) {
        err_sys("udp_client error");
    }
    const int on = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_PKTINFO, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }
    dg_cli(sockfd, (SA *)&servaddr, servlen);
}

void dg_cli(int sockfd, struct sockaddr *pservaddr, socklen_t servlen) {
    char sendline[MAXLINE], recvline[MAXLINE + 1];
    struct sockaddr_storage reply_addr;

    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen) < 0) {
            err_sys("sendto error");
        }
        socklen_t len = sizeof(reply_addr);
        ssize_t n = recvfrom(sockfd, recvline, MAXLINE, 0, (SA *)&reply_addr, &len);
        if (n < 0) {
            err_sys("recvfrom error");
        }
        recvline[n] = 0;
        printf("reply from %s", sock_ntop((SA *)&reply_addr, len));
        if (len != servlen || memcmp(pservaddr, &reply_addr, len) != 0) {
            printf(" ignored\n");
        } else {
            printf(": %s", recvline);
        }
    }
    if (ferror(stdin)) {
        err_sys("fgets error");
    }
}
