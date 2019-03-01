#include "unp.h"
#include <netdb.h>

void str_cli(FILE *fp, int sockfd);

int tcp_connect(const char *host, const char *serv);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: tcpcli <hostname/IPaddress> <service/port>");
    }
    int sockfd = tcp_connect(argv[1], argv[2]);
    str_cli(stdin, sockfd);
}

int tcp_connect(const char *host, const char *serv) {
    struct addrinfo hint, *res, *addrptr;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;

    int n = getaddrinfo(host, serv, &hint, &res);
    if (n != 0) {
        err_quit("tcp_connect error for %s, %s: %s", host, serv, gai_strerror(n));
    }

    int sockfd;
    for (addrptr = res; addrptr != NULL; addrptr = addrptr->ai_next) {
        sockfd = socket(addrptr->ai_family, addrptr->ai_socktype, addrptr->ai_protocol);
        if (sockfd < 0) {
            continue;
        }
        if (connect(sockfd, addrptr->ai_addr, addrptr->ai_addrlen) == 0) {
            break;
        }
        close(sockfd);
    }
    freeaddrinfo(res);
    if (addrptr == NULL) {
        err_quit("tcp_connect error for %s, %s", host, serv);
    }
    return sockfd;
}
