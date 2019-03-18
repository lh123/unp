#include "unp.h"
#include <pthread.h>

static void sig_int(int signo);
void *doit(void *arg);
void web_child(int fd);
void pr_cpu_time(void);

int main(int argc, char **argv) {
    int listenfd;
    if (argc == 2) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else if (argc == 3) {
        listenfd = tcp_listen(argv[1], argv[2]);
    } else {
        err_quit("usage: serv06 [ <host> ] <port#>");
    }
    if (listenfd < 0) {
        err_quit("tcp_listen error");
    }
    struct sockaddr_storage cliaddr;

    struct sigaction act;
    act.sa_handler = sig_int;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGINT, &act, NULL) < 0) {
        err_sys("sigaction error");
    }

    for (;;) {
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
        if (connfd < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                err_sys("accept error");
            }
        }
        pthread_t tid;
        pthread_create(&tid, NULL, &doit, (void *)(size_t)connfd);
    }
}

void *doit(void *arg) {
    pthread_detach(pthread_self());
    int fd = (int)(size_t)arg;
    web_child(fd);
    close(fd);
    return NULL;
}

static void sig_int(int signo) {
    pr_cpu_time();
    exit(0);
}

