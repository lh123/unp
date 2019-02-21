#include "unp.h"

void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen);

int main(int argc, char **argv) {
    struct sockaddr_in servaddr;
    int sockfd;

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
    
    if (connect(sockfd, pservaddr, servlen) < 0) {
        err_sys("connect error");
    }

    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (write(sockfd, sendline, strlen(sendline)) < 0) {
            err_sys("write error");
        }

        if ((n = read(sockfd, recvline, MAXLINE)) < 0) {
            err_sys("read error");
        }
        recvline[n] = 0;

        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs to stdout error");
        }
    }

    if (ferror(fp)) {
        err_sys("fgets error");
    }
}
