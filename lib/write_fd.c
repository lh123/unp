#include "unp.h"

ssize_t write_fd(int fd, void *ptr, size_t nbytes, int sendfd) {
    struct msghdr msg;
    struct iovec iov[1];

    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;

    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    
    control_un.cm.cmsg_len = CMSG_LEN(sizeof(int));
    control_un.cm.cmsg_level = SOL_SOCKET;
    control_un.cm.cmsg_type = SCM_RIGHTS;
    *(int *)CMSG_DATA(&control_un.cm) = sendfd;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    return sendmsg(fd, &msg, 0);
}
