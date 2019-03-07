#include "unp.h"
#include <netdb.h>

int udp_server(const char *host, const char *serv, socklen_t *addrlenp) {
    struct addrinfo hint, *res, *addrptr;
    int sockfd;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_flags = AI_PASSIVE;

    int n = getaddrinfo(host, serv, &hint, &res);
    if (n != 0) {
        err_quit("udp_server error for %s, %s: %s", host, serv, gai_strerror(n));
    }

    for (addrptr = res; addrptr != NULL; addrptr = addrptr->ai_next) {
        sockfd = socket(addrptr->ai_family, addrptr->ai_socktype, addrptr->ai_protocol);
        if (sockfd < 0) {
            continue;
        }
        if (bind(sockfd, addrptr->ai_addr, addrptr->ai_addrlen) == 0) {
            break;
        }
        close(sockfd);
    }
    if (addrptr == NULL) {
        err_quit("udp_server error for %s, %s", host, serv);
    }
    if (addrlenp != NULL) {
        *addrlenp = addrptr->ai_addrlen;
    }
    freeaddrinfo(res);
    return sockfd;
}
