#include "unp.h"

ssize_t read_cred(int fd, void *ptr, size_t nbytes, struct ucred *credptr) {
    struct msghdr msg;
    struct iovec iov[1];

    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(struct ucred))];
    } control_un;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un);
    msg.msg_flags = 0;

    ssize_t n = recvmsg(fd, &msg, 0);
    if (n < 0) {
        return n;
    }
    if (credptr) {
        credptr->gid = 0;
    }
    if (credptr && msg.msg_controllen > 0) {
        struct cmsghdr *cmptr = &control_un.cm;
        if (cmptr->cmsg_len < CMSG_LEN(sizeof(struct ucred))) {
            err_quit("control length = %d", (int)cmptr->cmsg_len);
        }
        if (cmptr->cmsg_level != SOL_SOCKET) {
            err_quit("control level != SOL_SOCKET");
        }
        if (cmptr->cmsg_type != SCM_CREDENTIALS) {
            err_quit("control type != SCM_CREDENTIALS");
        }
        memcpy(credptr, CMSG_DATA(cmptr), sizeof(struct ucred));
    }

    return n;
}
