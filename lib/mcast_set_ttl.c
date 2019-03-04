#include "unp.h"

int mcast_set_ttl(int sockfd, int val) {
    switch (sockfd_to_family(sockfd)) {
    case AF_INET: {
        u_char ttl = val;
        return setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    }
    case AF_INET6: {
        int hop = val;
        return setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hop, sizeof(hop));
    }
    default:
        errno = EAFNOSUPPORT;
        return -1;
    }
}
