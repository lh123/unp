#include "unp.h"
#include <netdb.h>

int tcp_connect(const char *host, const char *service) {
    struct addrinfo hint, *res, *addrptr;
    int sockfd;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;

    int n = getaddrinfo(host, service, &hint, &res);
    if (n != 0) {
        err_quit("tcp_connect error for %s, %s: %s", host, service, gai_strerror(n));
    }
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
        err_quit("tcp_connect error for %s, %s", host, service);
    }
    return sockfd;
}
