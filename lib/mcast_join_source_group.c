#include "unp.h"
#include <net/if.h>

int mcast_join_source_group(int sockfd, const struct sockaddr *src, socklen_t srclen, const struct sockaddr *grp, socklen_t grplen, const char *ifname, u_int ifindex) {
#ifdef MCAST_JOIN_SOURCE_GROUP
    struct group_source_req gsreq;
    if (ifindex > 0) {
        gsreq.gsr_interface = ifindex;
    } else if (ifname != NULL) {
        if ((gsreq.gsr_interface = if_nametoindex(ifname)) == 0) {
            errno = ENXIO;
            return -1;
        }
    } else {
        gsreq.gsr_interface = 0;
    }
    if (srclen > sizeof(gsreq.gsr_source) || grplen > sizeof(gsreq.gsr_group)) {
        errno = EINVAL;
        return -1;
    }
    memcpy(&gsreq.gsr_source, src, srclen);
    memcpy(&gsreq.gsr_group, grp, grplen);
    return setsockopt(sockfd, family_to_level(grp->sa_family), MCAST_JOIN_SOURCE_GROUP, &gsreq, sizeof(gsreq));
#else
    if (grp->sa_family != AF_INET) {
        errno = EAFNOSUPPORT;
        return -1;
    }
    struct ip_mreq_source mreq;
    struct ifreq ireq;
    if (ifindex > 0) {
        if (if_indextoname(ifindex, ireq.ifr_name) == NULL) {
            errno = ENXIO;
            return -1;
        }
        goto doioctl;
    } else if (ifname != NULL) {
        strncpy(ireq.ifr_name, ifname, IFNAMSIZ);
doioctl:
        if (ioctl(sockfd, SIOCGIFADDR, &ireq) < 0) {
            return -1;
        }
        memcpy(&mreq.imr_interface, &((struct sockaddr_in *)&ireq.ifr_addr)->sin_addr, sizeof(struct in_addr));
    } else {
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    }
    return setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq));
#endif
}
