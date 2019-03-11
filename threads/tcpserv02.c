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
        err_quit("usage: tcpserv02 [hostname/IPaddress] <service/port>");
    }

    struct sockaddr_storage cliaddr;
    for (;;) {
        socklen_t len = sizeof(cliaddr);
        int *iptr = malloc(sizeof(int)); // malloc 不可重入，但是线程安全的
        *iptr = accept(listenfd, (SA *)&cliaddr, &len);
        if (*iptr < 0) {
            err_sys("accept error");
        }
        pthread_t tid;
        pthread_create(&tid, NULL, doit, iptr);
    }
}

static void *doit(void *arg) {
    int connfd = *(int *)arg;
    free(arg);

    pthread_detach(pthread_self());
    str_echo(connfd);
    close(connfd);
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
