#include "unp.h"
#include "sum.h"

void sig_cld(int signo);
void str_echo(int sockfd);

int main(int argc, char **argv) {
    struct sigaction act;
    act.sa_handler = sig_cld;
    act.sa_flags = SA_RESTART;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("sigaction SIGCHLD error");
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

    for (;;) {
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if (connfd < 0) {
            if (errno == EINTR) {
                continue;
            }
            err_sys("accept error");
        }
        int pid = fork();
        if (pid < 0) {
            err_sys("fork error");
        } else if (pid == 0) {
            close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        close(connfd);
    }
}

void sig_cld(int signo) {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("chlid %d terminated\n", pid);
    }
}

void str_echo(int sockfd) {
    ssize_t n;
    struct args args;
    struct result result;
    for (;;) {
        if ((n = readn(sockfd, &args, sizeof(args))) < 0) {
            err_sys("readn error");
        } else if (n == 0) {
            return;
        }
        result.sum = args.arg1 + args.arg2;
        if (writen(sockfd, &result, sizeof(result)) < 0) {
            err_sys("writen error");
        }
    }
}
