#include "unp.h"
#include <net/if.h>

int mcast_leave_source_group(int sockfd, const struct sockaddr *src, socklen_t srclen, const struct sockaddr *grp, socklen_t grplen) {
#ifdef MCAST_LEAVE_SOURCE_GROUP
    struct group_source_req gsreq;
    if (srclen > sizeof(gsreq.gsr_source) || grplen > sizeof(gsreq.gsr_group)) {
        errno = EINVAL;
        return -1;
    }
    gsreq.gsr_interface = 0;
    memcpy(&gsreq.gsr_source, src, srclen);
    memcpy(&gsreq.gsr_group, grp, grplen);
    return setsockopt(sockfd, family_to_level(grp->sa_family), MCAST_LEAVE_SOURCE_GROUP, &gsreq, sizeof(gsreq));
#else
    if (grp->sa_family != AF_INET) {
        errno = EAFNOSUPPORT;
        return -1;
    }
    struct ip_mreq_source mreq;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    memcpy(&mreq.imr_sourceaddr, src, srclen);
    memcpy(&mreq.imr_multiaddr, grp, grplen);
    return setsockopt(sockfd, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq));
#endif
}
