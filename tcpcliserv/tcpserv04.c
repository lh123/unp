#include "unp.h"

void sig_chld(int signo);
void str_echo(int sockfd);

int main(int argc, char **argv) {
    struct sigaction act;
    act.sa_handler = sig_chld;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("sigaction for SIGCHLD error");
    }

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        err_sys("socket error");
    }
    struct sockaddr_in cliaddr, servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }
    if (listen(listenfd, LISTENQ) < 0) {
        err_sys("listen error");
    }

    for (;;) {
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
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
            str_echo(connfd);
            exit(0);
        }
        close(connfd);
    }
}

void str_echo(int sockfd) {
    ssize_t n;
    char buf[MAXLINE];

again:
    while ((n = read(sockfd, buf, MAXLINE)) > 0) {
        if (writen(sockfd, buf, n) < 0) {
            err_sys("write error");
        }
    }
    if (n < 0 && errno == EINTR) {
        goto again;
    } else if (n < 0) {
        err_sys("str_echo: read error");
    }
}

void sig_chld(int signo) {
    printf("sig_clid received\n");
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("child %d terminated\n", pid);
    }
}
