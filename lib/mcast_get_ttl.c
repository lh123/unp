#include "unp.h"

int mcast_get_ttl(int sockfd) {
    switch (sockfd_to_family(sockfd)) {
    case AF_INET: {
        u_char ttl;
        socklen_t len = sizeof(ttl);
        if (getsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, &len) < 0) {
            return -1;
        }
        return ttl;
    }
    case AF_INET6: {
        int hop;
        socklen_t len = sizeof(hop);
        if (getsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &hop, &len) < 0) {
            return -1;
        }
        return hop;
    }
    default:
        errno = EAFNOSUPPORT;
        return -1;
    }
}
