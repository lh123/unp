#include "unp.h"
#include <netdb.h>

void dg_echo(int sockfd);

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: udpserv01 <port>");
    }
    struct sockaddr_in servaddr, grpaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        err_sys("socket error");
    }
    if (bind(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }

    memset(&grpaddr, 0, sizeof(grpaddr));
    grpaddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, "224.0.0.1", &grpaddr.sin_addr) != 1) {
        err_quit("inet_pton error");
    }
    if (mcast_join(sockfd, (SA *)&grpaddr, sizeof(grpaddr), NULL, 0) < 0) {
        err_sys("mcast_join error");
    }
    dg_echo(sockfd);
}

void dg_echo(int sockfd) {
    struct sockaddr_storage cliaddr;
    char recvline[MAXLINE];

    for (;;) {
        socklen_t clilen = sizeof(cliaddr);
        ssize_t n = recvfrom(sockfd, recvline, MAXLINE, 0, (SA *)&cliaddr, &clilen);
        if (n < 0) {
            err_sys("recvfrom error");
        }
        if (sendto(sockfd, recvline, n, 0, (SA *)&cliaddr, clilen) < 0) {
            err_sys("sendto error");
        }
    }
}
