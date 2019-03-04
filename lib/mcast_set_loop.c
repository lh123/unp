#include "unp.h"

int mcast_set_loop(int sockfd, int onoff) {
    switch (sockfd_to_family(sockfd)) {
    case AF_INET: {
        u_char loop = onoff;
        return setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    }
    case AF_INET6: {
        u_int loop = onoff;
        return setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loop, sizeof(loop));
    }
    default:
        errno = EAFNOSUPPORT;
        return -1;
    }
}
