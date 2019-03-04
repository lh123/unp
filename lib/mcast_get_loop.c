#include "unp.h"

int mcast_get_loop(int sockfd) {
    switch (sockfd_to_family(sockfd)) {
    case AF_INET: {
        u_char loop;
        socklen_t len = sizeof(loop);
        if (getsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, &len) < 0) {
            return -1;
        }
        return loop;
    }
    case AF_INET6: {
        u_int loop;
        socklen_t len = sizeof(loop);
        if (getsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loop, &len) < 0) {
            return -1;
        }
        return loop;
    }
    default:
        errno = EAFNOSUPPORT;
        return -1;
    }
}
