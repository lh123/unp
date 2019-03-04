#include "unp.h"

int mcast_leave(int sockfd, const struct sockaddr *grp, socklen_t grplen) {
#ifdef MCAST_LEAVE_GROUP
    struct group_req greq;
    greq.gr_interface = 0;
    if (grplen > sizeof(greq.gr_group)) {
        errno = EINVAL;
        return -1;
    }
    memcpy(&greq.gr_group, grp, grplen);
    return setsockopt(sockfd, family_to_level(grp->sa_family), MCAST_LEAVE_GROUP, &greq, sizeof(greq));
#else
    switch (grp->sa_family) {
    case AF_INET: {
        struct ip_mreq mreq;
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        memcpy(&mreq.imr_multiaddr, &((const struct sockaddr_in *)grp)->sin_addr, sizeof(struct in_addr));
        return setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
    }
    case AF_INET6: {
        struct ipv6_mreq mreq6;
        mreq6.ipv6mr_interface = 0;
        memcpy(&mreq6.ipv6mr_multiaddr, &((const struct sockaddr_in6 *)grp)->sin6_addr, sizeof(struct in6_addr));
        return setsockopt(sockfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &mreq6, sizeof(mreq6));
    }
    default:
        errno = EAFNOSUPPORT;
        return -1;
    }
#endif
}
