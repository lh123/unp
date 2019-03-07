#include "unp.h"

ssize_t recvfrom_flags(int fd, void *ptr, size_t nbytes, int *flagsp, struct sockaddr *sa, socklen_t *salenptr, struct in_pktinfo *pktp) {
    struct msghdr msg;
    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
    } control_un;
    
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    msg.msg_flags = 0;
    msg.msg_name = sa;
    msg.msg_namelen = *salenptr;

    struct iovec iov[1];
    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    ssize_t n = recvmsg(fd, &msg, *flagsp);
    if (n < 0) {
        return n;
    }
    *salenptr = msg.msg_namelen;
    if (pktp) {
        memset(pktp, 0, sizeof(struct in_pktinfo));
    }
    *flagsp = msg.msg_flags;
    if (msg.msg_controllen < sizeof(struct cmsghdr) || (msg.msg_flags & MSG_CTRUNC) || pktp == NULL) {
        return n;
    } 
    struct cmsghdr *cmptr;
    for (cmptr = CMSG_FIRSTHDR(&msg); cmptr != NULL; cmptr = CMSG_NXTHDR(&msg, cmptr)) {
        if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_PKTINFO) {
            struct in_pktinfo *info = (struct in_pktinfo *)CMSG_DATA(cmptr);
            memcpy(pktp, info, sizeof(struct in_pktinfo));
            break;
        }
    }
    return n;
}
