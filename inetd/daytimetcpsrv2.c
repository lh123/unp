#include "unp.h"
#include <time.h>
#include <netdb.h>

static int tcp_listen(const char *host, char *serv);

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        err_quit("usage: daytimetcpsrv2 [ <host> ] <service or port>");
    }
    daemon_init(argv[0], 0);
    int listenfd;
    if (argc == 2) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else {
        listenfd = tcp_listen(argv[1], argv[2]);
    }

    struct sockaddr_storage cliaddr;
    char buff[MAXLINE];
    for (;;) {
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if (connfd < 0) {
            if (errno == ECONNRESET) {
                continue;
            } else {
                err_sys("accept error");
            }
        }

        err_msg("connection from %s", sock_ntop((struct sockaddr *)&cliaddr, clilen));

        time_t ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        if (write(connfd, buff, strlen(buff)) < 0) {
            err_sys("write error");
        }
        close(connfd);
    }
}

static int tcp_listen(const char *host, char *serv) {
    struct addrinfo hint, *res, *addrptr;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    int n = getaddrinfo(host, serv, &hint, &res);
    if (n != 0) {
        err_quit("tcp_listen error for %s, %s: %s", host, serv, gai_strerror(n));
    }

    int listenfd;
    for (addrptr = res; addrptr != NULL; addrptr = addrptr->ai_next) {
        listenfd = socket(addrptr->ai_family, addrptr->ai_socktype, addrptr->ai_protocol);
        if (listenfd < 0) {
            continue;
        }
        if (bind(listenfd, addrptr->ai_addr, addrptr->ai_addrlen) == 0) {
            if (listen(listenfd, LISTENQ) == 0) {
                break;
            }
        }
        close(listenfd);
    }

    if (addrptr == NULL) {
        err_quit("tcp_listen error for %s, %s", host, serv);
    }
    freeaddrinfo(res);
    
    return listenfd;
}
