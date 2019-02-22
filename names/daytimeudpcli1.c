#include "unp.h"
#include <netdb.h>

int udp_client(const char *host, const char *serv, struct sockaddr *saptr, socklen_t *lenptr);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: daytimeudpcli1 <hostname/IPaddress> <service/port#>");
    }
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);
    int sockfd = udp_client(argv[1], argv[2], (struct sockaddr *)&sa, &salen);

    printf("sending to %s\n", sock_ntop((struct sockaddr *)&sa, salen));

    if (sendto(sockfd, "", 1, 0, (struct sockaddr *)&sa, salen) < 0) {
        err_sys("sendto error");
    }

    char recvline[MAXLINE + 1];
    int n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
    recvline[n] = 0;

    if (fputs(recvline, stdout) == EOF) {
        err_sys("fputs error");
    }
}

int udp_client(const char *host, const char *serv, struct sockaddr *saptr, socklen_t *lenptr) {
    struct addrinfo *res, *addrptr, hint;
    int sockfd;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_DGRAM;

    int n = getaddrinfo(host, serv, &hint, &res);
    if (n != 0) {
        err_quit("udp_client error for %s, %s: %s", host, serv, gai_strerror(n));
    }

    for (addrptr = res; addrptr != NULL; addrptr = addrptr->ai_next) {
        sockfd = socket(addrptr->ai_family, addrptr->ai_socktype, addrptr->ai_protocol);
        if (sockfd >= 0) {
            break;
        }
    }

    if (addrptr == NULL) {
        err_sys("udp_client error for %s, %s", host, serv);
    }
    if (saptr != NULL) {
        if (*lenptr >= addrptr->ai_addrlen) {
            *lenptr = addrptr->ai_addrlen;
            memcpy(saptr, addrptr->ai_addr, addrptr->ai_addrlen);
        }
    }
    freeaddrinfo(res);
    return sockfd;
}
