#include "icmpd.h"
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

char *sock_ntop_host(struct sockaddr *addr, socklen_t socklen);

int readable_v4(void) {
    struct sockaddr_in from, dest;
    socklen_t len = sizeof(from);
    char buf[MAXLINE];
    char srcstr[INET_ADDRSTRLEN], dststr[INET_ADDRSTRLEN];

    ssize_t n = recvfrom(fd4, buf, MAXLINE, 0, (SA *)&from, &len);
    if (n < 0) {
        err_sys("recvfrom error");
    }
    printf("%d bytes ICMPv4 from %s:", (int)n, sock_ntop_host((SA *)&from, len));
    struct ip *ip = (struct ip *)buf;
    int hlen1 = ip->ip_hl << 2;
    struct icmp *icmp = (struct icmp *)(buf + hlen1);
    int icmplen = n - hlen1;
    if (icmplen < 8) {
        err_quit("icmplen (%d) < 8", icmplen);
    }
    printf(" type = %d, code = %d\n", icmp->icmp_type, icmp->icmp_code);
    if (icmp->icmp_type == ICMP_UNREACH ||
        icmp->icmp_type == ICMP_TIMXCEED ||
        icmp->icmp_type == ICMP_SOURCEQUENCH) {
        if (icmplen < 8 + 20 + 8) { // 8 icmp 20 ip 8 udp
            err_quit("icmplen (%d) < 8 + 20 + 8", icmplen);
        }
        struct ip *hip = (struct ip *)(buf + hlen1 + 8);
        int hlen2 = hip->ip_hl << 2;
        printf("\tsrcip = %s, dstip = %s, proto = %d\n",
                inet_ntop(AF_INET, &hip->ip_src, srcstr, sizeof(srcstr)),
                inet_ntop(AF_INET, &hip->ip_dst, dststr, sizeof(dststr)),
                hip->ip_p);
        if (hip->ip_p == IPPROTO_UDP) {
            struct udphdr *udp = (struct udphdr *)(buf + hlen1 + 8 + hlen2);
            int sport = udp->uh_sport;
            int i;
            for (i = 0; i <= maxi; i++) {
                if (client[i].connfd >= 0 &&
                    client[i].family == AF_INET &&
                    client[i].lport == sport) {
                    memset(&dest, 0, sizeof(dest));
                    dest.sin_family = AF_INET;
                    memcpy(&dest.sin_addr, &hip->ip_dst, sizeof(struct in_addr));
                    dest.sin_port = udp->uh_dport;

                    struct icmpd_err icmpd_err;
                    icmpd_err.icmpd_type = icmp->icmp_type;
                    icmpd_err.icmpd_code = icmp->icmp_code;
                    icmpd_err.icmpd_len = sizeof(struct sockaddr_in);
                    memcpy(&icmpd_err.icmpd_dest, &dest, sizeof(dest));

                    icmpd_err.icmpd_errno = EHOSTUNREACH;
                    if (icmp->icmp_type == ICMP_UNREACH) {
                        if (icmp->icmp_code == ICMP_UNREACH_PORT) {
                            icmpd_err.icmpd_errno = ECONNREFUSED;
                        } else if (icmp->icmp_code == ICMP_UNREACH_NEEDFRAG) {
                            icmpd_err.icmpd_errno = EMSGSIZE;
                        }
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
