#include "unp.h"

void sig_chld(int signo);
void str_echo(int sockfd);

int main(int argc, char **argv) {
    struct sigaction act;
    act.sa_handler = sig_chld;
    act.sa_flags = SA_RESTART;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("sigaction for SIGCHLD error");
    }

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        err_sys("socket error");
    }
    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }

    if (listen(listenfd, LISTENQ) < 0) {
        err_sys("listen error");
    }
    socklen_t servlen = sizeof(servaddr);
    if (getsockname(listenfd, (struct sockaddr *)&servaddr, &servlen) < 0) {
        err_sys("getsockname error");
    }

    char addrbuff[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &servaddr.sin_addr, addrbuff, INET6_ADDRSTRLEN) <= 0) {
        err_sys("inet_ntop error");
    }
    printf("address: %s:%d\n", addrbuff, ntohs(servaddr.sin_port));
    
    for (;;) {
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if (connfd < 0) {
            if (errno == EINTR) {
                continue;
            }
            err_sys("accept error");
        }
        pid_t cldpid = fork();
        if (cldpid < 0) {
            err_sys("fork error");
        } else if (cldpid == 0) {
            close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        close(connfd);
    }
}

void str_echo(int sockfd) {
    char line[MAXLINE];
    ssize_t n;
    long arg1, arg2;
    for (;;) {
        if ((n = readline(sockfd, line, sizeof(line))) < 0) {
            err_sys("readline error");
        } else if (n == 0) {
            return;
        }
        if (sscanf(line, "%ld%ld", &arg1, &arg2) == 2) {
            snprintf(line, sizeof(line), "%ld\n", arg1 + arg2);
        } else {
            snprintf(line, sizeof(line), "input error\n");
        }
        n = strlen(line);
        if (writen(sockfd, line, n) < 0) {
            err_sys("writen error");
        }
    }
}

void sig_chld(int signo) {
    for (;;) {
        pid_t pid = waitpid(-1, NULL, WNOHANG);
        if (pid <= 0) {
            break;
        }
        printf("child %d terminated\n", pid);
    }
}
