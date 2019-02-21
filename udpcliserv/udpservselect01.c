#include "unp.h"
#include <sys/select.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void sig_chld(int signal);
static void str_echo(int sockfd);

int main(int argc, char **argv) {
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // create listening tcp socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        err_sys("socket error");
    }
    const int reuseaddr = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0) {
        err_sys("setsockopt error");
    }

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }

    if (listen(listenfd, LISTENQ) < 0) {
        err_sys("listen error");
    }

    // create udp socket
    int udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpfd < 0) {
        err_sys("socket error");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(udpfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }

    struct sigaction act;
    act.sa_handler = sig_chld;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("sigaction(SIGCHLD) error");
    }

    fd_set rset;
    FD_ZERO(&rset);
    int maxfdp1 = MAX(listenfd, udpfd) + 1;
    
    for (;;) {
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);

        int nready = select(maxfdp1, &rset, NULL, NULL, NULL);
        if (nready < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                err_sys("select error");
            }
        }
        
        struct sockaddr cliaddr;
        socklen_t len;
        if (FD_ISSET(listenfd, &rset)) {
            len = sizeof(cliaddr);
            int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
            if (connfd < 0) {
                err_sys("accept error");
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
        if (FD_ISSET(udpfd, &rset)) {
            len = sizeof(cliaddr);
            char mesg[MAXLINE];
            int n = recvfrom(udpfd, mesg, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
            if (n < 0) {
                err_sys("recvfrom error");
            }
            if (sendto(udpfd, mesg, n, 0, (struct sockaddr *)&cliaddr, len) != n) {
                err_sys("sendto error");
            }
        }
    }
}

static void sig_chld(int signal) {
    int stat;
    pid_t pid;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("child %d exited\n", (int)pid);
    }
}

static ssize_t my_writen(int fd, const void *buf, size_t size) {
    ssize_t nleft = size;
    const char *ptr = buf;
    while (nleft > 0) {
        int n = write(fd, ptr, nleft);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }
        nleft -= n;
        ptr += n;
    }
    return size;
}

static void str_echo(int sockfd) {
    char mesg[MAXLINE];
    int n;
    while ((n = read(sockfd, mesg, MAXLINE)) > 0) {
        if (my_writen(sockfd, mesg, n) < 0) {
            err_sys("write error");
        }
    }
    if (n < 0) {
        err_sys("read error");
    }
}
