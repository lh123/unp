#include "unp.h"
#include <sys/un.h>

void dg_echo(int sockfd, SA *cliaddr, socklen_t clilen);

int main(int argc, char **argv) {
    int sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        err_sys("socket error");
    }
    struct sockaddr_un servaddr, cliaddr;
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, UNIXDG_PATH);

    unlink(UNIXSTR_PATH);
    if (bind(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }

    dg_echo(sockfd, (SA *)&cliaddr, sizeof(cliaddr));
}

void dg_echo(int sockfd, SA *cliaddr, socklen_t clilen) {
    char recvline[MAXLINE];

    for (;;) {
        socklen_t len = clilen;
        ssize_t n = recvfrom(sockfd, recvline, MAXLINE, 0, cliaddr, &len);
        if (n < 0) {
            err_sys("recvfrom error");
        }

        if (sendto(sockfd, recvline, n, 0, cliaddr, len) < 0) {
            err_sys("sendto error");
        }
    }
}
