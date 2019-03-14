#include "unp.h"

void inet6_srcrt_print(void *ptr);

void dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen) {
    const int on = 1;
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_RECVRTHDR, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }
    union {
        struct cmsghdr cmsg;
        char control[MAXLINE];
    } control_un;
    struct msghdr msg;
    char mesg[MAXLINE];

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = pcliaddr;
    
    struct iovec iov[1];
    iov[0].iov_base = mesg;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control_un.control;

    for (;;) {
        msg.msg_namelen = clilen;
        msg.msg_controllen = sizeof(control_un.control);
        iov[0].iov_len = sizeof(mesg);
        ssize_t n = recvmsg(sockfd, &msg, 0);
        if (n < 0) {
            err_sys("recvmsg error");
        }
        struct cmsghdr *cmsg;
        for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            if (cmsg->cmsg_level == IPPROTO_IPV6 && cmsg->cmsg_type == IPV6_RTHDR) {
                inet6_srcrt_print(CMSG_DATA(cmsg));
                inet6_rth_reverse(CMSG_DATA(cmsg), CMSG_DATA(cmsg));
            }
        }
        iov[0].iov_len = n;
        if (sendmsg(sockfd, &msg, 0) < 0) {
            err_sys("sendmsg error");
        }
    }
}

