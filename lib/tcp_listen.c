#include "unp.h"
#include <netdb.h>

int tcp_listen(const char *host, const char *service) {
    struct addrinfo hint, *res, *aptr;
    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    int err = getaddrinfo(host, service, &hint, &res);
    if (err != 0) {
        err_quit("tcp_listen error for %s, %s: %s", host, service, gai_strerror(err));
    }

    int sockfd;
    for (aptr = res; aptr != NULL; aptr = aptr->ai_next) {
        sockfd = socket(aptr->ai_family, aptr->ai_socktype, aptr->ai_protocol);
        if (sockfd < 0) {
            continue;
        }
        if (bind(sockfd, aptr->ai_addr, aptr->ai_addrlen) == 0) {
            if (listen(sockfd, LISTENQ) == 0) {
                break;
            }
        }
        close(sockfd);
    }
    freeaddrinfo(res);

    if (aptr == NULL) {
        err_quit("tcp_listen error for %s, %s", host, service);
    }
    return sockfd;
}
