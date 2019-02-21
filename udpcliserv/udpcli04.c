#include "unp.h"

// datagrams to send
#define NDG 2000
// length of each datagram
#define DGLEN 1400

typedef struct sockaddr SA;

void dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen);

int main(int argc, char **argv) {
    if (argc != 2) {
        err_sys("usage: udpcli <IPaddress>");
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        err_sys("inet_pton error");
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        err_sys("socket error");
    }

    dg_cli(stdin, sockfd, (SA *)&servaddr, sizeof(servaddr));
}

void dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen) {
    int i;
    char sendline[DGLEN];
    for (i = 0; i < NDG; i++) {
        if (sendto(sockfd, sendline, DGLEN, 0, pservaddr, servlen) < 0) {
            err_sys("sendto error");
        }
    }
}
