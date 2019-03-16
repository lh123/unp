#include "icmpd.h"

int sock_get_port(SA *paddr);
int sock_bind_wild(int sockfd, int family); 

int readable_conn(int i) {
    int unixfd = client[i].connfd;
    int recvfd = -1;
    
    char c;
    char control[CMSG_SPACE(sizeof(int))];
    
    struct iovec iov[1];
    iov[0].iov_base = &c;
    iov[0].iov_len = 1;

    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    ssize_t n = recvmsg(unixfd, &msg, 0);
    if (n < 0) {
        err_sys("recvmsg error");
    } else if (n == 0) {
        err_msg("client %d terminated, recvfd = %d", i, recvfd);
        goto clientdone;
    } else if (n != 1) {
        err_quit("n = %d, expected 1", (int)n);
    }
    struct cmsghdr *cmptr;
    for (cmptr = CMSG_FIRSTHDR(&msg); cmptr != NULL; cmptr = CMSG_NXTHDR(&msg, cmptr)) {
        if (cmptr->cmsg_len != CMSG_LEN(sizeof(int))) {
            continue;
        }
        if (cmptr->cmsg_level == SOL_SOCKET && cmptr->cmsg_type == SCM_RIGHTS) {
            recvfd = *(int *)CMSG_DATA(cmptr);
            break;
        }
    }
    if (recvfd < 0) {
        err_msg("recvmsg did not return descriptor");
        goto clienterr;
    }
    struct sockaddr_storage ss;
    socklen_t len = sizeof(ss);
    if (getsockname(recvfd, (SA *)&ss, &len) < 0) {
        err_ret("getsockname error");
        goto clienterr;
    }
    client[i].family = ss.ss_family;
    if ((client[i].lport = sock_get_port((SA *)&ss)) == 0) {
        client[i].lport = sock_bind_wild(recvfd, client[i].family);
        if (client[i].lport <= 0) {
            err_ret("error binding ephemeral port");
            goto clienterr;
        }
    }
    if (write(unixfd, "1", 1) < 0) {
        err_sys("write error");
    }
    close(recvfd);
    return --nready;

clienterr:
    if (write(unixfd, "0", 1) < 0) {
        err_sys("write error");
    }
clientdone:
    close(unixfd);
    if (recvfd >= 0) {
        close(recvfd);
    }
    FD_CLR(unixfd, &allset);
    client[i].connfd = -1;
    return --nready;
}
