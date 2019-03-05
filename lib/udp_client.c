#include "unp.h"
#include <netdb.h>

int udp_client(const char *host, const char *service, struct sockaddr *addr, socklen_t *addrlen) {
    struct addrinfo hint, *res, *aiptr;
    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_flags = AI_PASSIVE;

    int err = getaddrinfo(host, service, &hint, &res);
    if (err != 0) {
        err_quit("udp_client error for %s, %s: %s", host, service, gai_strerror(err));
    }

    int sockfd;
    for (aiptr = res; aiptr != NULL; aiptr = aiptr->ai_next) {
        sockfd = socket(aiptr->ai_family, aiptr->ai_socktype, aiptr->ai_protocol);
        if (sockfd < 0) {
            continue;
        }
        break;
    }
    if (addr != NULL && addrlen != NULL) {
        if (*addrlen >= aiptr->ai_addrlen) {
            memcpy(addr, aiptr->ai_addr, aiptr->ai_addrlen);
            *addrlen = aiptr->ai_addrlen;
        } else {
            sockfd = -1;
            errno = ENOBUFS;
            goto out;
        }
    }
out:
    freeaddrinfo(res);
    return sockfd;
}
