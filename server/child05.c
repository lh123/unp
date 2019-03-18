#include "unp.h"
#include "child.h"

Child *cptr;
void web_child(int fd);
ssize_t recv_fd(int sockfd, char *ptr, size_t nbytes, int *fd);

pid_t child_make(int i, int listenfd) {
    int sockfd[2];
    pid_t pid;

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd) < 0) {
        err_sys("socketpair error");
    }

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid > 0) {
        close(sockfd[1]);
        cptr[i].child_pid = pid;
        cptr[i].child_pipefd = sockfd[0];
        cptr[i].child_status = 0;
        return pid;
    }

    if (dup2(sockfd[1], STDERR_FILENO) != STDERR_FILENO) {
        err_sys("dup2 error");
    }
    close(sockfd[0]);
    close(sockfd[1]);
    close(listenfd);
    child_main(i, listenfd);
    return pid;
}

void child_main(int i, int listenfd) {
    printf("serv05 child %ld starting\n", (long)getpid());
    for (;;) {
        char c;
        int connfd;
        ssize_t n = recv_fd(STDERR_FILENO, &c, 1, &connfd);
        if (n == 0) {
            err_quit("recv_fd returned 0");
        }
        if (connfd < 0) {
            err_quit("no descriptor from recv_fd");
        }
        web_child(connfd);
        close(connfd);
        if (write(STDERR_FILENO, "", 1) != 1) { // tell parent we're ready again
            err_sys("write error");
        }
    }
}

ssize_t recv_fd(int sockfd, char *ptr, size_t nbytes, int *fd) {
    struct msghdr msg;

    char control[CMSG_SPACE(sizeof(int))];

    struct iovec iov[1];
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;

    memset(&msg, 0, sizeof(msg));
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    ssize_t nread = recvmsg(sockfd, &msg, 0);
    if (nread < 0) {
        err_sys("recvmsg error");
    }
    struct cmsghdr *cmptr;
    *fd = -1;
    for (cmptr = CMSG_FIRSTHDR(&msg); cmptr != NULL; cmptr = CMSG_NXTHDR(&msg, cmptr)) {
        if (cmptr->cmsg_len < CMSG_LEN(sizeof(int))) {
            continue;
        }
        if (cmptr->cmsg_level == SOL_SOCKET && cmptr->cmsg_type == SCM_RIGHTS) {
            *fd = *(int *)CMSG_DATA(cmptr);
            break;
        }
    }
    return nread;
}
