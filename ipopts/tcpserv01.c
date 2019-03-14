#include "unp.h"

void inet_srcrt_print(u_char *ptr, int len);

void str_echo(int sockfd);
void sig_cld(int signo);

int main(int argc, char **argv) {
    int listenfd = tcp_listen(NULL, SERV_PORT_STR);
    if (listenfd < 0) {
        err_sys("udp_server error");
    }
    struct sigaction act;
    act.sa_handler = sig_cld;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("sigaction error");
    }

    u_char opts[44];
    for (;;) {
        int connfd = accept(listenfd, NULL, NULL);
        if (connfd < 0) {
            continue;
        }
        socklen_t len = 44;
        if (getsockopt(connfd, IPPROTO_IP, IP_OPTIONS, opts, &len) < 0) {
            err_sys("getsockopt error");
        }
        if (len > 0) {
            printf("received IP options, len = %d\n", len);
            inet_srcrt_print(opts, len);
        }
        pid_t pid = fork();
        if (pid < 0) {
            err_sys("fork error");
        } else if (pid == 0) {
            // child
            close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        close(connfd);
    }
}

void str_echo(int sockfd) {
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

void sig_cld(int signo) {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("\nchild %d exited\n", (int)pid);
    }
    if (pid < 0 && errno != ECHILD) {
        err_sys("waitpid error");
    }
}
