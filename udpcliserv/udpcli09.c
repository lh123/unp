#include "unp.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: udpcli <IPaddress>");
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

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("connect error");
    }

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    if (getsockname(sockfd, (struct sockaddr *)&cliaddr, &clilen) < 0) {
        err_sys("getsockname error");
    }

    char addrbuf[INET_ADDRSTRLEN];
    printf("local address %s:%d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, addrbuf, sizeof(addrbuf)), ntohs(servaddr.sin_port));
}
