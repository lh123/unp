#include "unp.h"

static void sig_chld(int signo);
static void sig_int(int signo);
void web_child(int sockfd);

int main(int argc, char **argv) {
    int listenfd;
    if (argc == 2) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else if (argc == 3) {
        listenfd = tcp_listen(argv[1], argv[2]);
    } else {
        err_quit("usage: serv01 [ <host> ] <port#>");
    }
    struct sockaddr_storage cliaddr;
    struct sigaction act;
    act.sa_handler = sig_chld;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    act.sa_handler = sig_int;
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

        pid_t childpid = fork();
        if (childpid < 0) {
            err_sys("fork error");
        } else if (childpid == 0) {
            close(listenfd);
            web_child(connfd);
            exit(0);
        }
        close(connfd);
    }
}

static void sig_int(int signo) {
    void pr_cpu_time(void);

    pr_cpu_time();
    exit(0);
}

static void sig_chld(int signo) {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
        ;
    if (pid < 0) {
        if (errno != ECHILD) {
            err_sys("waitpid error");
        }
    }
}
