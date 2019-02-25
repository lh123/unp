#include "unp.h"
#include <sys/un.h>

static void sig_cld(int signo);
static void str_echo(int sockfd);

int main(int argc, char **argv) {
    int listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (listenfd < 0) {
        err_sys("socket error");
    }
    unlink(UNIXSTR_PATH);
    
    struct sockaddr_un cliaddr, servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strncpy(servaddr.sun_path, UNIXSTR_PATH, sizeof(servaddr.sun_path) - 1);

    if (bind(listenfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }

    if (listen(listenfd, LISTENQ) < 0) {
        err_sys("listen error");
    }

    struct sigaction act;
    act.sa_handler = sig_cld;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("sigaction SIGCHLD error");
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
            str_echo(connfd);
            exit(0);
        }
        close(connfd);
    }
}

static void sig_cld(int signo) {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("child %d exit\n", (int)pid);
    }
}

static void str_echo(int sockfd) {
    char recvline[MAXLINE];
    ssize_t n;

    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        if (writen(sockfd, recvline, n) < 0) {
            err_sys("write error");
        }
    }
    if (n < 0) {
        err_sys("read error");
    }
}
