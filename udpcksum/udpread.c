#include "udpcksum.h"
#include <net/ethernet.h>

uint16_t in_cksum(uint16_t *addr, int len);
struct udpiphdr *udp_check(char *ptr, int len);

struct udpiphdr *udp_read(void) {
    for (;;) {
        int len;
        char *ptr = next_pcap(&len);
        struct ether_header *eptr;
        switch (datalink) {
        case DLT_NULL:
            return udp_check(ptr + 4, len - 4);
        case DLT_EN10MB:
            eptr = (struct ether_header *)ptr;
            if (ntohs(eptr->ether_type) != ETHERTYPE_IP) {
                err_quit("Ethernet type %x not IP", ntohs(eptr->ether_type));
            }
            // sizeof(ehter_header) = 14
            return udp_check(ptr + 14, len - 14);
        case DLT_SLIP: // SLIP header = 24 bytes
            return udp_check(ptr + 24, len - 24);
        case DLT_PPP: // PPP header = 24 bytes
            return udp_check(ptr + 24, len - 24);
        default:
            err_quit("unsupported datalink (%d)", datalink);
        }
    }
}

struct udpiphdr *udp_check(char *ptr, int len) {
    if (len < sizeof(struct ip) + sizeof(struct udphdr)) {
        err_quit("len = %d", len);
    }
    struct ip *ip = (struct ip *)ptr;
    if (ip->ip_v != IPVERSION) {
        err_quit("ip_v = %d", ip->ip_v);
    }
    int hlen = ip->ip_hl << 2;
    if (hlen < sizeof(struct ip)) {
        err_quit("ip_hl = %d", ip->ip_hl);
    }
    if (len < hlen + sizeof(struct udphdr)) {
        err_quit("len = %d, hlen = %d", len, hlen);
    }
    if ((ip->ip_sum = in_cksum((uint16_t *)ip, hlen)) != 0) {
        err_quit("ip checksum error %x", ip->ip_sum);
    }
    if (ip->ip_p == IPPROTO_UDP) {
        return (struct udpiphdr *)ip;
    } else {
        err_quit("not a UDP packet");
    }
}
