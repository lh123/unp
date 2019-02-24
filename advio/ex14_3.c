#include "unp.h"
#include <netdb.h>

int tcp_connect(const char *host, const char *serv);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: daytimetcpcli <hostname/IPaddress> <service/port#>");
    }

    struct sockaddr_storage ss;
    int sockfd = tcp_connect(argv[1], argv[2]);
    socklen_t len = sizeof(ss);

    if (getpeername(sockfd, (SA *)&ss, &len) < 0) {
        err_sys("getpeername errr");
    }
    printf("connected to %s\n", sock_ntop((SA *)&ss, len));

    ssize_t n;
    char recvline[MAXLINE + 1];
    while ((n = recv(sockfd, recvline, MAXLINE, MSG_PEEK)) > 0) {
        int npend;
        if (ioctl(sockfd, FIONREAD, &npend) < 0) {
            err_sys("ioctl FIONREAD error");
        }
        printf("%ld bytes from PEEK, %d bytes pending\n", n, npend);
        
        if ((n = read(sockfd, recvline, MAXLINE)) < 0) {
            err_sys("read error");
        }
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }

    if (n < 0) {
        err_sys("read error");
    }
}

int tcp_connect(const char *host, const char *serv) {
    struct addrinfo hint, *res, *ptr;
    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_family = AF_UNSPEC;

    int err = getaddrinfo(host, serv, &hint, &res);
    if (err != 0) {
        err_quit("tcp_connect error for %s, %s: %s", host, serv, gai_strerror(err));
    }

    int sockfd;
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next) {
        sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sockfd < 0) {
            continue;
        }
        if (connect(sockfd, ptr->ai_addr, ptr->ai_addrlen) == 0) {
            break;
        }
        close(sockfd);
    }
    freeaddrinfo(res);
    
    if (ptr == NULL) {
        err_quit("tcp_connect error for %s, %s", host, serv);
    }
    return sockfd;
}
