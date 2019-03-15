#include "ping.h"

char *sock_ntop_host(struct sockaddr *addr, socklen_t socklen);

void proc_v6(char *ptr, ssize_t len, struct msghdr *msg, struct timeval *tvrecv) {
    struct icmp6_hdr *icmp6 = (struct icmp6_hdr *)ptr;
    if (len < 8) {
        return; // malformed packet
    }
    if (icmp6->icmp6_type == ICMP6_ECHO_REPLY) {
        if (ntohs(icmp6->icmp6_id) != pid) {
            return;
        }
        if (len < 16) {
            return;
        }
        struct timeval *tvsend = (struct timeval *)(icmp6 + 1);
        tv_sub(tvrecv, tvsend);
        double rtt = tvrecv->tv_sec * 1000.0 + tvrecv->tv_usec / 1000.0;
        int hlim = -1;
        struct cmsghdr *cmsg;
        for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL; cmsg = CMSG_NXTHDR(msg, cmsg)) {
            if (cmsg->cmsg_level == IPPROTO_IPV6 && cmsg->cmsg_type == IPV6_HOPLIMIT) {
                hlim = *(uint32_t *)CMSG_DATA(cmsg);
                break;
            }
        }
        printf("%d bytes from %s: seq=%u, hlim=", (int)len, sock_ntop_host(pr->sarecv, pr->salen), ntohs(icmp6->icmp6_seq));
        if (hlim == -1) {
            printf("???");
        } else {
            printf("%d", hlim);
        }
        printf(", rtt=%.3f ms\n", rtt);
    } else if (verbose) {
        printf("  %d bytes from %s: type = %d, code = %d\n", (int)len, sock_ntop_host(pr->sarecv, pr->salen), icmp6->icmp6_type, icmp6->icmp6_code);
    }
}
