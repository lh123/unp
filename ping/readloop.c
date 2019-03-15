#include "ping.h"
#include <sys/time.h>

void readloop(void) {
    sockfd = socket(pr->sasend->sa_family, SOCK_RAW, pr->icmpproto);
    if (sockfd < 0) {
        err_sys("socket error");
    }
    setuid(getuid()); // don't need special permissions any more
    if (pr->finit) {
        (*pr->finit)();
    }
    int size = 60 * 1024;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    sig_alrm(SIGALRM); // send first packet

    char recvbuf[BUFSIZE];
    union {
        struct cmsghdr cmsg;
        char control[BUFSIZE];
    } msg_control;
    
    struct iovec iov[1];
    iov[0].iov_base = recvbuf;
    iov[0].iov_len = sizeof(recvbuf);
    
    struct msghdr msg;
    msg.msg_name = pr->sarecv;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = msg_control.control;
    for (;;) {
        msg.msg_namelen = pr->salen;
        msg.msg_controllen = sizeof(msg_control.control);
        ssize_t n = recvmsg(sockfd, &msg, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                err_sys("recvmsg error");
            }
        }
        struct timeval tval;
        gettimeofday(&tval, NULL);
        (*pr->fproc)(recvbuf, n, &msg, &tval);
    }
}
