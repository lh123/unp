#include "unp.h"

int mcast_get_if(int sockfd) {
    switch (sockfd_to_family(sockfd)) {
    case AF_INET: {
        struct in_addr addr;
        socklen_t len = sizeof(addr);
        if (getsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &addr, &len) < 0) {
            return -1;
        }
        return ntohl(addr.s_addr);
    }
    case AF_INET6: {
        u_int index;
        socklen_t len = sizeof(index);
        if (getsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &index, &len) < 0) {
            return -1;
        }
        return index;
    }
    default:
        errno = EAFNOSUPPORT;
        return -1;
    }
}
