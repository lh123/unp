#include "unp.h"
#include <net/if.h>

int mcast_set_if(int sockfd, const char *ifname, u_int ifindex) {
    switch (sockfd_to_family(sockfd)) {
    case AF_INET: {
        struct in_addr addr;
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
            memcpy(&addr, &((struct sockaddr_in *)&ireq.ifr_addr)->sin_addr, sizeof(addr));
        } else {
            // clear prev, set to default
            addr.s_addr = htonl(INADDR_ANY);
        }
        return setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &addr, sizeof(addr));
    }
    case AF_INET6: {
        u_int index;
        if (ifindex > 0) {
            index = ifindex;
        } else if (ifname != NULL) {
            if ((index = if_nametoindex(ifname)) == 0) {
                errno = ENXIO;
                return -1;
            }
        } else {
            // clear prev, set to default
            index = 0;
        }
        return setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &index, sizeof(index));
    }
    default:
        errno = EAFNOSUPPORT;
        return -1;
    }
}
