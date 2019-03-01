#include "unp.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: tcpcli <IPaddress>");
    }
    struct sockaddr_in servaddr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    int err = inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    if (err == 0) {
        err_quit("inet_pton error: invaild ip address: %s", argv[1]);
    } else if (err < 0) {
        err_sys("inet_pton error");
    }

    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("connect error");
    }

    struct linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)) < 0) {
        err_sys("setsockopt error");
    }
    close(sockfd);
}
