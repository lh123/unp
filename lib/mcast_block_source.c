#include "unp.h"

int mcast_block_source(int sockfd, const struct sockaddr *src, socklen_t srclen, const struct sockaddr *grp, socklen_t grplen) {
#ifdef MCAST_BLOCK_SOURCE
    struct group_source_req gsreq;
    if (srclen > sizeof(gsreq.gsr_source) || grplen > sizeof(gsreq.gsr_group)) {
        errno = EINVAL;
        return -1;
    }
    gsreq.gsr_interface = 0;
    memcpy(&gsreq.gsr_source, src, srclen);
    memcpy(&gsreq.gsr_group, grp, grplen);
    return setsockopt(sockfd, family_to_level(grp->sa_family), MCAST_BLOCK_SOURCE, &gsreq, sizeof(gsreq));
#else
    struct ip_mreq_source mreq;
    if (grp->sa_family != AF_INET) {
        errno = EINVAL;
        return -1;
    }
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    memcpy(&mreq.imr_sourceaddr, &((const struct sockaddr_in *)src)->sin_addr, sizeof(struct in_addr));
    memcpy(&mreq.imr_multiaddr, &((const struct sockaddr_in *)grp)->sin_addr, sizeof(struct in_addr));
    return setsockopt(sockfd, IPPROTO_IP, IP_BLOCK_SOURCE, &mreq, sizeof(mreq));
#endif
}
