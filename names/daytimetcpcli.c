#include "unp.h"
#include <netdb.h>

int tcp_connect(const char *host, const char *service);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: daytimetcpcli <hostname/IPaddress> <service/port#>");
    }
    int sockfd = tcp_connect(argv[1], argv[2]);
    if (sockfd < 0) {
        err_sys("tcp_connect error");
    }
    struct sockaddr_storage ss;
    socklen_t len = sizeof(ss);
    if (getpeername(sockfd, (struct sockaddr *)&ss, &len) < 0) {
        err_sys("getpeername error");
    }
    printf("connected to %s\n", sock_ntop((struct sockaddr *)&ss, len));
    
    int n;
    char recvline[MAXLINE + 1];
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
}

int tcp_connect(const char *host, const char *service) {
    struct addrinfo hint, *res, *paddr;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;

    int n = getaddrinfo(host, service, &hint, &res);
    if (n != 0) {
        err_quit("tcp_connect error for %s, %s: %s", host, service, gai_strerror(n));
    }
    int sockfd;
    for (paddr = res; paddr != NULL; paddr = paddr->ai_next) {
        sockfd = socket(paddr->ai_family, paddr->ai_socktype, paddr->ai_protocol);
        if (sockfd < 0) {
            continue;
        }
        if (connect(sockfd, paddr->ai_addr, paddr->ai_addrlen) == 0) {
            break;
        }
        close(sockfd);
    }
    freeaddrinfo(res);
    if (paddr == NULL) {
        err_quit("tcp_connect error for %s, %s", host, service);
    }
    return sockfd;
}
