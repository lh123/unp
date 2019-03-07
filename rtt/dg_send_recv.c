#include "unprtt.h"
#include <setjmp.h>

#define RTT_DEBUG

static struct rtt_info rttinfo;
static int rttinit = 0;

struct hdr {
    uint32_t seq;
    uint32_t ts;
} sendhdr, recvhdr;

static void sig_alrm(int signo);
static sigjmp_buf jmpbuf;

ssize_t dg_send_recv(int fd, const void *outbuff, size_t outbytes, void *inbuff, size_t inbytes, const SA *destaddr, socklen_t destlen) {
    if (rttinit == 0) {
        rtt_init(&rttinfo);
        rttinit = 1;
        rtt_d_flag = 1;
    }

    struct iovec iovsend[2], iovrecv[2];

    struct msghdr msgsend, msgrecv;
    sendhdr.seq++;
    msgsend.msg_name = destaddr;
    msgsend.msg_namelen = destlen;
    msgsend.msg_iov = iovsend;
    msgsend.msg_iovlen = 2;
    iovsend[0].iov_base = &sendhdr;
    iovsend[0].iov_len = sizeof(struct hdr);
    iovsend[1].iov_base = outbuff;
    iovsend[1].iov_len = outbytes;

    msgrecv.msg_name = NULL;
    msgrecv.msg_namelen = 0;
    msgrecv.msg_iov = iovrecv;
    msgrecv.msg_iovlen = 2;
    iovrecv[0].iov_base = &recvhdr;
    iovrecv[0].iov_len = sizeof(struct hdr);
    iovrecv[1].iov_base = inbuff;
    iovrecv[1].iov_len = inbytes;

    struct sigaction act;
    act.sa_handler = sig_alrm;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGALRM, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    rtt_newpack(&rttinfo);

sendagain:
    sendhdr.ts = rtt_ts(&rttinfo);
    if (sendmsg(fd, &msgsend, 0) < 0) {
        err_sys("sendmsg error");
    }
    alarm(rtt_start(&rttinfo));
    if (sigsetjmp(jmpbuf, 1) != 0) {
        if (rtt_timeout(&rttinfo) < 0) {
            err_msg("dg_send_recv: no response from server, giving up");
            rttinit = 0;
            errno = ETIMEDOUT;
            return -1;
        }
        goto sendagain;
    }
    ssize_t n;
    do {
        n = recvmsg(fd, &msgrecv, 0);
    } while (n < sizeof(struct hdr) || recvhdr.seq != sendhdr.seq);
    alarm(0);

    rtt_stop(&rttinfo, rtt_ts(&rttinfo) - recvhdr.ts);
    return n - sizeof(struct hdr);
}

static void sig_alrm(int signo) {
    siglongjmp(jmpbuf, 1);
}
