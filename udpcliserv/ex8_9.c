#include "unp.h"
#include <sys/poll.h>
#include <netinet/ip.h>

static void sig_cld(int signo);
static void str_echo(int sockfd);
static void str_echo_udp(int sockfd);

int main(int argc, char **argv) {
    int tcpfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpfd < 0) {
        err_sys("socket error");
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    const int on = 1;
    if (setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }

    if (bind(tcpfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }

    if (listen(tcpfd, LISTENQ) < 0) {
        err_sys("listen error");
    }

    int udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpfd < 0) {
        err_sys("socket error");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (setsockopt(udpfd, SOL_IP, IP_PKTINFO, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }

    if (bind(udpfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }

    struct sigaction act;
    act.sa_handler = sig_cld;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("signal SIGCHLD error");
    }

    struct pollfd readfd[2];
    readfd[0].fd = tcpfd;
    readfd[0].events = POLLRDNORM;
    readfd[1].fd = udpfd;
    readfd[1].events = POLLRDNORM;

    for (;;) {
        int nready = poll(readfd, 2, -1);
        if (nready < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                err_sys("poll error");
            }
        }
        if (readfd[0].revents & (POLLRDNORM | POLLERR)) {
            struct sockaddr_in cliaddr;
            socklen_t clilen = sizeof(cliaddr);
            int connfd = accept(tcpfd, (struct sockaddr *)&cliaddr, &clilen);
            if (connfd < 0) {
                err_ret("accept error");
            } else {
                pid_t pid = fork();
                if (pid < 0) {
                    err_sys("fork error");
                } else if (pid == 0) {
                    close(tcpfd);
                    str_echo(connfd);
                    exit(0);
                }
                close(connfd);
            }
        }
        if (readfd[1].revents & (POLLRDNORM | POLLERR)) {
            str_echo_udp(udpfd);
        }
    }
}

static void sig_cld(int signo) {
    int stat;
    pid_t pid;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("child %d exit\n", (int)pid);
    }
}

static int my_writen(int fd, const void *buf, size_t n) {
    size_t nleft = n;
    const char *ptr = buf;
    while (nleft > 0) {
        int nwrite = write(fd, ptr, nleft);
        if (nwrite < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }
        nleft -= nwrite;
        ptr += nwrite;
    }
    return n;
}

static void str_echo(int sockfd) {
    char recvline[MAXLINE];
    int n;
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        if (my_writen(sockfd, recvline, n) < 0) {
            err_sys("write error");
        }
    }

    if (n < 0) {
        err_sys("read error");
    }
}

static void str_echo_udp(int sockfd) {
    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
    } control_u;
    char recvline[MAXLINE];
    struct msghdr msg;
    struct iovec iov[1];
    struct sockaddr_in cliaddr;

    iov[0].iov_base = recvline;
    iov[0].iov_len = MAXLINE;

    msg.msg_control = control_u.control;
    msg.msg_controllen = sizeof(control_u.control);
    msg.msg_flags = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_name = &cliaddr;
    msg.msg_namelen = sizeof(cliaddr);

    int n = recvmsg(sockfd, &msg, 0);
    if (n < 0) {
        err_sys("recvmsg error");
    }

    struct cmsghdr *ptr;
    struct in_pktinfo *info = NULL;
    for (ptr = CMSG_FIRSTHDR(&msg); ptr != NULL; ptr = CMSG_NXTHDR(&msg, ptr)) {
        if (ptr->cmsg_level == IPPROTO_IP && ptr->cmsg_type == IP_PKTINFO) {
            info = (struct in_pktinfo *)CMSG_DATA(ptr);
            break;
        }
    }
    if (info == NULL) {
        printf("can't get dst info");
        return;
    }

    char addrbuf[INET_ADDRSTRLEN];
    printf("server: router addr: %s\n", inet_ntop(AF_INET, &info->ipi_spec_dst, addrbuf, INET_ADDRSTRLEN));
    printf("server: header addr: %s\n", inet_ntop(AF_INET, &info->ipi_addr, addrbuf, INET_ADDRSTRLEN));
    printf("server: source addr: %s\n", inet_ntop(AF_INET, &cliaddr.sin_addr, addrbuf, INET_ADDRSTRLEN));

    if (sendto(sockfd, recvline, n, 0, (struct sockaddr *)&cliaddr, msg.msg_namelen) < 0) {
        err_sys("sendto error");
    }
}
