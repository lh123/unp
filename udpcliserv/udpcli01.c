#include "unp.h"

void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen);

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2) {
        err_quit("usage: udpcli <IPaddress>");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        err_sys("inet_pton error");
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        err_sys("socket error");
    }

    dg_cli(stdin, sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen) {
    int n;
    char sendline[MAXLINE], recvline[MAXLINE + 1];

    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen) < 0) {
            err_sys("sendto error");
        }

        if ((n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL)) < 0) {
            err_sys("recvfrom error");
        }

        recvline[n] = 0;
        
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
}
