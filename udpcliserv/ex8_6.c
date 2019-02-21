#include "unp.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: sock <src> <dst>");
    }

    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if (inet_pton(AF_INET, argv[2], &servaddr.sin_addr) != 1) {
        err_sys("inet_pton error");
    }
    
    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(0);
    if (inet_pton(AF_INET, argv[1], &cliaddr.sin_addr) != 1) {
        err_sys("inet_pton error");
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        err_sys("socket error");
    }

    const int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) {
        err_sys("setsockopt error");
    }

    if (bind(sockfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0) {
        err_sys("bind error");
    }

    char sendline[MAXLINE];
    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            err_sys("sendto error");
        }
        struct sockaddr_in srcaddr;
        socklen_t srclen = sizeof(srcaddr);
        int n = recvfrom(sockfd, sendline, MAXLINE, 0, (struct sockaddr *)&srcaddr, &srclen);
        if (n < 0) {
            err_sys("recvfrom error");
        }
        char addrbuff[INET_ADDRSTRLEN];
        printf("client: src addr: %s\n", inet_ntop(AF_INET, &srcaddr.sin_addr, addrbuff, INET_ADDRSTRLEN));

        if (fputs(sendline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }

    if (ferror(stdin)) {
        err_sys("fgets error");
    }
}
