#include "unp.h"
#include <time.h>
#include <netdb.h>

int tcp_listen(const char *host, const char *serv, socklen_t *paddrlen);

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: daytimetcpsrv1 <service or port#>");
    }
    int listenfd = tcp_listen(NULL, argv[1], NULL);
    char buf[MAXLINE];

    for (;;) {
        struct sockaddr_storage cliaddr;
        socklen_t len = sizeof(cliaddr);
        int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
        if (connfd < 0) {
            continue;
        }
        printf("connection from %s\n", sock_ntop((struct sockaddr *)&cliaddr, len));

        time_t ticks = time(NULL);
        snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));

        if (write(connfd, buf, strlen(buf)) != strlen(buf)) {
            err_sys("write error");
        }
        close(connfd);
    }
}

int tcp_listen(const char *host, const char *serv, socklen_t *paddrlen) {
    struct addrinfo hint, *res, *paddr;
    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    hint.ai_family = AF_UNSPEC;

    int n = getaddrinfo(host, serv, &hint, &res);
    if (n != 0) {
        err_quit("tcp_listen error for %s, %s: %s", host, serv, gai_strerror(n));
    }

    int listenfd;
    for (paddr = res; paddr != NULL; paddr = paddr->ai_next) {
        listenfd = socket(paddr->ai_family, paddr->ai_socktype, paddr->ai_protocol);
        if (listenfd < 0) {
            continue;
        }
        const int optval = 1;
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
            err_sys("setsockopt error");
        }
        if (bind(listenfd, paddr->ai_addr, paddr->ai_addrlen) == 0) {
            if (listen(listenfd, LISTENQ) == 0) {
                break;
            }
        }
        close(listenfd);
    }
    if (paddr == NULL) {
        err_sys("tcp_listen error for %s, %s", host, serv);
    }

    if (paddrlen) {
        *paddrlen = paddr->ai_addrlen;
    }
    freeaddrinfo(res);

    return listenfd;
}
