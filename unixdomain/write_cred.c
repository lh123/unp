#include "unp.h"

ssize_t write_cred(int fd, void *ptr, size_t nbyte) {
    struct msghdr msg;
    struct iovec iov[1];

    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(struct ucred))];
    } control_un;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbyte;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;

    control_un.cm.cmsg_len = CMSG_LEN(sizeof(struct ucred));
    control_un.cm.cmsg_level = SOL_SOCKET;
    control_un.cm.cmsg_type = SCM_CREDENTIALS;

    struct ucred *credptr = (struct ucred *)CMSG_DATA(&control_un.cm);
    credptr->uid = getuid();
    credptr->pid = getpid();
    credptr->gid = getgid();

    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);

    return sendmsg(fd, &msg, 0);
}
