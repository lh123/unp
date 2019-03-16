#include "icmpd.h"
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

char *sock_ntop_host(struct sockaddr *addr, socklen_t socklen);

int readable_v6(void) {
    struct sockaddr_in6 from, dest;
    socklen_t len = sizeof(from);
    char buf[MAXLINE];
    char srcstr[INET6_ADDRSTRLEN], dststr[INET6_ADDRSTRLEN];

    ssize_t n = recvfrom(fd6, buf, MAXLINE, 0, (SA *)&from, &len);
    if (n < 0) {
        err_sys("recvfrom error");
    }
    printf("%d bytes ICMPv6 from %s:", (int)n, sock_ntop_host((SA *)&from, len));
    struct icmp6_hdr *icmp6 = (struct icmp6_hdr *)buf;
    int icmp6len = n;
    if (icmp6len < 8) {
        err_quit("icmp6len (%d) < 8", icmp6len);
    }
    printf(" type = %d, code = %d\n", icmp6->icmp6_type, icmp6->icmp6_code);
    if (icmp6->icmp6_type == ICMP6_DST_UNREACH ||
        icmp6->icmp6_type == ICMP6_PACKET_TOO_BIG ||
        icmp6->icmp6_type == ICMP6_TIME_EXCEEDED) {
        if (icmp6len < 8 + 8) { // 8 icmp 8 udp
            err_quit("icmp6len (%d) < 8 + 8", icmp6len);
        }
        struct ip6_hdr *hip6 = (struct ip6_hdr *)(buf + 8); // skip icmp header
        int hlen2 = sizeof(struct ip6_hdr);
        printf("\tsrcip = %s, dstip = %s, proto = %d\n",
                inet_ntop(AF_INET6, &hip6->ip6_src, srcstr, sizeof(srcstr)),
                inet_ntop(AF_INET6, &hip6->ip6_dst, dststr, sizeof(dststr)),
                hip6->ip6_nxt);
        if (hip6->ip6_nxt == IPPROTO_UDP) {
            struct udphdr *udp = (struct udphdr *)(buf + 8 + hlen2); // skip ICMPv6 and IPv6
            int sport = udp->uh_sport;
            int i;
            for (i = 0; i <= maxi; i++) {
                if (client[i].connfd >= 0 &&
                    client[i].family == AF_INET &&
                    client[i].lport == sport) {
                    memset(&dest, 0, sizeof(dest));
                    dest.sin6_family = AF_INET6;
                    memcpy(&dest.sin6_addr, &hip6->ip6_dst, sizeof(struct in6_addr));
                    dest.sin6_port = udp->uh_dport;

                    struct icmpd_err icmpd_err;
                    icmpd_err.icmpd_type = icmp6->icmp6_type;
                    icmpd_err.icmpd_code = icmp6->icmp6_code;
                    icmpd_err.icmpd_len = sizeof(struct sockaddr_in6);
                    memcpy(&icmpd_err.icmpd_dest, &dest, sizeof(dest));

                    icmpd_err.icmpd_errno = EHOSTUNREACH;
                    if (icmp6->icmp6_type == ICMP6_DST_UNREACH &&
                        icmp6->icmp6_code == ICMP6_DST_UNREACH_NOPORT) {
                        icmpd_err.icmpd_errno = ECONNREFUSED;
                    }
                    if (icmp6->icmp6_type == ICMP6_PACKET_TOO_BIG) {
                        icmpd_err.icmpd_errno = EMSGSIZE;
                    }
                    if (write(client[i].connfd, &icmpd_err, sizeof(icmpd_err)) < 0) {
                        err_sys("write error");
                    }
                }
            }
        }
    }
    return --nready;
}
