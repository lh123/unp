#include "ping.h"

char *sock_ntop_host(struct sockaddr *addr, socklen_t socklen);

void proc_v4(char *ptr, ssize_t len, struct msghdr *msg, struct timeval *tvrecv) {
    struct ip *ip = (struct ip *)ptr;
    int hlen1 = ip->ip_hl << 2; // (* 4) length of IP header
    if (ip->ip_p != IPPROTO_ICMP) {
        return;
    }
    struct icmp *icmp = (struct icmp *)(ptr + hlen1);
    int icmplen = len - hlen1;
    if (icmplen < 8) {
        return; // malformed packet
    }
    if (icmp->icmp_type == ICMP_ECHOREPLY) {
        if (ntohs(icmp->icmp_id) != pid) {
            return; // not a response to our ECHO_REQUEST
        }
        if (icmplen < 16) {
            return; // not enough data to use
        }
        struct timeval *tvsend = (struct timeval *)icmp->icmp_data;
        tv_sub(tvrecv, tvsend);
        double rtt = tvrecv->tv_sec * 1000.0 + tvrecv->tv_usec / 1000.0;
        printf("%d bytes from %s: seq=%u, ttl=%d, rtt=%.3f ms\n", icmplen, sock_ntop_host(pr->sarecv, pr->salen), ntohs(icmp->icmp_seq), ip->ip_ttl, rtt);
    } else if (verbose) {
        printf("  %d bytes from %s: type = %d, code = %d\n", icmplen, sock_ntop_host(pr->sarecv, pr->salen), icmp->icmp_type, icmp->icmp_code);
    }
}
