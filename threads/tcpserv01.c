#include "unp.h"
#include <pthread.h>

static void *doit(void *arg);
static void str_echo(int sockfd);

int main(int argc, char **argv) {
    int listenfd;
    if (argc == 2) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else if (argc == 3) {
        listenfd = tcp_listen(argv[1], argv[2]);
    } else {
        err_quit("usage: tcpserv01 [hostname/IPaddress] <service/port>");
    }
    struct sockaddr_storage cliaddr;
    for (;;) {
        socklen_t len = sizeof(cliaddr);
        int connfd = accept(listenfd, (SA *)&cliaddr, &len);
        pthread_t tid;
        int err = pthread_create(&tid, NULL, doit, (void *)(size_t)connfd);
        if (err != 0) {
            err_exit(err, "pthread_create error");
        }
    }
}

static void *doit(void *arg) {
    int err = pthread_detach(pthread_self());
    if (err != 0) {
        err_exit(err, "pthread_detach error");
    }
    str_echo((int)(size_t)arg);
    close((int)(size_t)arg);
    return NULL;
}

static void str_echo(int sockfd) {
    char recvline[MAXLINE];
    ssize_t n;
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        if (writen(sockfd, recvline, n) < 0) {
            err_sys("writen error");
        }
    }
    if (n < 0) {
        err_sys("read error");
    }
}
