#include "unp.h"
#include <netinet/tcp.h>

int main(int argc, char **argv) {
    int sockfd, rcvbuf, mss;
    socklen_t len;
    struct sockaddr_in servaddr;
    
    if (argc != 2) {
        err_quit("usage: rcvbuf <IPaddress>");
    }
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_sys("socket error");
    }
    len = sizeof(rcvbuf);
    if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &len) == -1) {
        err_sys("getsockopt error");
    }
    len = sizeof(mss);
    if (getsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, &mss, &len) == -1) {
        err_sys("getsockopt error");
    }
    printf("default: SO_RCVBUF = %d, MSS = %d\n", rcvbuf, mss);
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        err_sys("inet_pton error");
    }
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("connect error");
    }

    len = sizeof(rcvbuf);
    if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &len) == -1) {
        err_sys("getsockopt error");
    }
    len = sizeof(mss);
    if (getsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, &mss, &len) == -1) {
        err_sys("getsockopt error");
    }
    printf("after connect: SO_RCVBUF = %d, MSS = %d\n", rcvbuf, mss);
}
