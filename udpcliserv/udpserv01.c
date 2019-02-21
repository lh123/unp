#include "unp.h"

void dg_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen);

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        err_sys("socket error");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }

    dg_echo(sockfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
}

void dg_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen) {
    socklen_t len;
    int n;
    char mesg[MAXLINE];
    for (;;) {
        len = clilen;
        if ((n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len)) < 0) {
            err_sys("recvfrom error");
        }
        if (sendto(sockfd, mesg, n, 0, pcliaddr, len) < 0) {
            err_sys("sendto error");
        }
    }

}
