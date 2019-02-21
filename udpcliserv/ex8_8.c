#include "unp.h"

#define MSG_SIZE 65508

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
    const int size = 100000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) < 0) {
        err_sys("setsockopt error");
    }
    if (sockfd < 0) {
        err_sys("socket error");
    }
    char msg[MSG_SIZE];
    if (sendto(sockfd, msg, MSG_SIZE, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("sendto error");
    }
} 
