#include "unp.h"
#include "child.h"

static int nchildren;
static void sig_int(int signo);
ssize_t send_fd(int sockfd, const char *ptr, size_t nbytes, int fd);
void pr_cpu_time(void);

int main(int argc, char **argv) {
    int listenfd;
    if (argc == 3) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else if (argc == 4) {
        listenfd = tcp_listen(argv[1], argv[2]);
    } else {
        err_quit("usage: serv05 [ <host> ] <port#> <#children>");
    }
    fd_set rset, masterset;
    FD_ZERO(&masterset);
    FD_SET(listenfd, &masterset);

    int maxfd = listenfd;
    struct sockaddr_storage cliaddr;
    nchildren = atoi(argv[argc - 1]);
    int navail = nchildren;
    cptr = calloc(nchildren, sizeof(Child));

    int i;
    for (i = 0; i < nchildren; i++) {
        child_make(i, listenfd);
        FD_SET(cptr[i].child_pipefd, &masterset);
        maxfd = maxfd > cptr[i].child_pipefd ? maxfd : cptr[i].child_pipefd;
    }
    struct sigaction act;
    act.sa_handler = sig_int;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGINT, &act, NULL) < 0) {
        err_sys("sigaction error");
    }

    for (;;) {
        rset = masterset;
        if (navail <= 0) {
            FD_CLR(listenfd, &rset);
        }
        int nsel = select(maxfd + 1, &rset, NULL, NULL, NULL); 
        if (nsel < 0 && errno == EINTR) {
            continue;
        }
        if (FD_ISSET(listenfd, &rset)) {
            socklen_t clilen = sizeof(cliaddr);
            int connfd = accept(listenfd, (SA *)&cliaddr, &clilen);

            for (i = 0; i < nchildren; i++) {
                if (cptr[i].child_status == 0) {
                    break;
                }
            }
            if (i == nchildren) {
                err_quit("no available children");
            }
            cptr[i].child_status = 1;
            cptr[i].child_count++;
            navail--;

            ssize_t n = send_fd(cptr[i].child_pipefd, "", 1, connfd);
            if (n != 1) {
                err_sys("send_fd error");
            }
            close(connfd);
            FD_SET(cptr[i].child_pipefd, &masterset);
            maxfd = maxfd > cptr[i].child_pipefd ? maxfd : cptr[i].child_pipefd;
            if (--nsel <= 0) {
                continue;
            }
        }
        for (i = 0; i < nchildren; i++) {
            if (cptr[i].child_status == 1 && FD_ISSET(cptr[i].child_pipefd, &rset)) {
                char c;
                ssize_t n = read(cptr[i].child_pipefd, &c, 1);
                if (n != 1) {
                    err_quit("read returned %d", (int)n);
                }
                FD_CLR(cptr[i].child_pipefd, &masterset);
                cptr[i].child_status = 0;
                navail++;
                if (--nsel <= 0) {
                    break;
                }
            }
        }
    }
}

ssize_t send_fd(int sockfd, const char *ptr, size_t nbytes, int fd) {
    struct msghdr msg;
    
    char control[CMSG_SPACE(sizeof(int))];
    struct cmsghdr *cmsg = (struct cmsghdr *)control;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    *(int *)CMSG_DATA(cmsg) = fd;

    struct iovec iov[1];
    iov[0].iov_base = (void *)ptr;
    iov[0].iov_len = nbytes;

    memset(&msg, 0, sizeof(msg));
    msg.msg_control = cmsg;
    msg.msg_controllen = sizeof(control);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    return sendmsg(sockfd, &msg, 0);
}

static void sig_int(int signo) {
    pr_cpu_time();
    int i;
    for (i = 0; i < nchildren; i++) {
        printf("%ld ", cptr[i].child_count);
    }
    exit(0);
}
