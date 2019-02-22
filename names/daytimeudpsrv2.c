#include "unp.h"
#include <netdb.h>
#include <time.h>

int udp_server(const char *host, const char *serv, socklen_t *addrlenp);

int main(int argc, char **argv) {
    int sockfd;
    if (argc == 2) {
        sockfd = udp_server(NULL, argv[1], NULL);
    } else if (argc == 3) {
        sockfd = udp_server(argv[1], argv[2], NULL);
    } else {
        err_quit("usage: daytimeudpsrv [ <host> ] <service or port>");
    }

    char buff[MAXLINE];
    for (;;) {
        struct sockaddr_storage cliaddr;
        socklen_t len = sizeof(cliaddr);
        ssize_t n = recvfrom(sockfd, buff, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            err_sys("recvfrom error");
        }
        printf("datagram from %s\n", sock_ntop((struct sockaddr *)&cliaddr, len));
        
        time_t ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        if (sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&cliaddr, len) < 0) {
            err_sys("sendto error");
        }
    }
}

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
