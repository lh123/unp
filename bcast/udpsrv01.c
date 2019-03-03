#include "unp.h"
#include <netdb.h>
#include <time.h>

void udp_daytime(int sockfd);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: udpsrv <hostname/IPaddress> <service/port>");
    }
    struct addrinfo hint, *res, *aiptr;
    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_flags = AI_PASSIVE;

    int err = getaddrinfo(argv[1], argv[2], &hint, &res);
    if (err != 0) {
        err_quit("getaddrinfo error for %s, %s: %s", argv[1], argv[2], gai_strerror(err));
    }

    int sockfd;
    for (aiptr = res; aiptr != NULL; aiptr = aiptr->ai_next) {
        sockfd = socket(aiptr->ai_family, aiptr->ai_socktype, aiptr->ai_protocol);
        if (sockfd < 0) {
            continue;
        }
        if (bind(sockfd, aiptr->ai_addr, aiptr->ai_addrlen) == 0) {
            break;
        }
        close(sockfd);
    }
    if (aiptr == NULL) {
        err_quit("socket error for %s, %s", argv[1], argv[2]);
    }
    freeaddrinfo(res);
    printf("listen on %s\n", sock_ntop(aiptr->ai_addr, aiptr->ai_addrlen));
    udp_daytime(sockfd);
}

void udp_daytime(int sockfd) {
    struct sockaddr_storage cliaddr;
    
    char buff[MAXLINE];
    for (;;) {
        socklen_t clilen = sizeof(cliaddr);
        ssize_t n = recvfrom(sockfd, buff, MAXLINE, 0, (SA *)&cliaddr, &clilen);
        if (n < 0) {
            err_sys("server: recvfrom error");
        }
        time_t ticks = time(NULL);
        n = snprintf(buff, MAXLINE, "%s", ctime(&ticks));
        if (sendto(sockfd, buff, n, 0, (SA *)&cliaddr, clilen) < 0) {
            err_sys("sendto error");
        }
    }
}
