#include "trace.h"

extern int gotalarm;

char *sock_ntop_host(struct sockaddr *addr, socklen_t socklen);

/*
 * return: -3 on timeout
 *         -2 on ICMP time exceeded in transit (caller keeps going)
 *         -1 on ICMP port unreachable (caller is done)
 *        >=0 return value is some other ICMP unreachable code
 */
int recv_v4(int seq, struct timeval *tv) {
    gotalarm = 0;
    alarm(3);
    struct udphdr *udp;
    int ret;
    for (;;) {
        if (gotalarm) {
            return -3;
        }
        socklen_t len = pr->salen;
        ssize_t n = recvfrom(recvfd, recvbuf, sizeof(recvbuf), 0, pr->sarecv, &len);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                err_sys("recvfrom error");
            }
        }
        struct ip *ip = (struct ip *)recvbuf;
        int hlen1 = ip->ip_hl << 2;
        struct icmp *icmp = (struct icmp *)(recvbuf + hlen1);
        int icmplen = n - hlen1;
        if (icmplen < 8) {
            continue;
        }
        if (icmp->icmp_type == ICMP_TIMXCEED && icmp->icmp_code == ICMP_TIMXCEED_INTRANS) {
            if (icmplen < 8 + sizeof(struct ip)) {
                continue;
            }
            struct ip *hip = (struct ip *)(recvbuf + hlen1 + 8);
            int hlen2 = hip->ip_hl << 2;
            if (icmplen < 8 + hlen2 + 4) {
                continue;
            }
            udp = (struct udphdr *)(recvbuf + hlen1 + 8 + hlen2);
            if (hip->ip_p == IPPROTO_UDP && 
                udp->uh_sport == htons(sport) &&
                udp->uh_dport == htons(dport + seq)) {
                ret = -2;
                break;
            }
        } else if (icmp->icmp_type == ICMP_UNREACH) {
            if (icmplen < 8 + sizeof(struct ip)) {
                continue;
            }
            struct ip *hip = (struct ip *)(recvbuf + hlen1 + 8);
            int hlen2 = hip->ip_hl << 2;
            if (icmplen < 8 + hlen2 + 4) {
                continue;
            }
            udp = (struct udphdr *)(recvbuf + hlen1 + 8 + hlen2);
            if (hip->ip_p == IPPROTO_UDP &&
                udp->uh_sport == htons(sport) &&
                udp->uh_dport == htons(dport + seq)) {
                if (icmp->icmp_code == ICMP_UNREACH_PORT) {
                    ret = -1;
                } else {
                    ret = icmp->icmp_code; // 0, 1, 2, ...
                }
                break;
            }
        }
        if (verbose) {
            printf(" (from %s: type = %d, code = %d)\n", sock_ntop_host(pr->sarecv, pr->salen), icmp->icmp_type, icmp->icmp_code);
        }
    }
    alarm(0);
    gettimeofday(tv, NULL);
    return ret;
}
