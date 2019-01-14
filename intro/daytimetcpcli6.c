#include "unp.h"
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: <IPaddress>");
    }
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0) {
        err_sys("socket error");
    }
    struct sockaddr_in6 servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(13);
    if (inet_pton(AF_INET6, argv[1], &servaddr.sin6_addr) <= 0) {
        err_quit("inet_pton error for %s", argv[1]);
    }
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("connect error");
    }
    int n;
    char recvline[MAXLINE + 1];
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
    if (n < 0) {
        err_sys("read error");
    }
}