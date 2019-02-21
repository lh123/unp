#include "unp.h"

void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen);

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2) {
        err_sys("usage: udpcli <IPaddress>");
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
    socklen_t len;
    struct sockaddr_storage preply_addr;

    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen) < 0) {
            err_sys("sendto error");
        }

        len = sizeof(preply_addr);
        if ((n = recvfrom(sockfd, recvline, MAXLINE, 0, (struct sockaddr *)&preply_addr, &len)) < 0) {
            err_sys("recvfrom error");
        }

        if (len != servlen || memcmp(&preply_addr, pservaddr, servlen) != 0) {
            printf("reply from %s (ignored)\n", sock_ntop((struct sockaddr *)&preply_addr, len));
            continue;
        }

        recvline[n] = 0; // null terminate
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }

    if (ferror(fp)) {
        err_sys("fgets error");
    }
    if (feof(fp)) {
        printf("eof detected\n");
    }
}
