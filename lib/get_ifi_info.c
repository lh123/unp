#include "unpifi.h"

struct ifi_info *get_ifi_info(int family, int doaliases) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int len = 100 * sizeof(struct ifreq);
    int lastlen = 0;
    struct ifconf ifc;
    char *buf;
    for (;;) {
        buf = malloc(len);
        ifc.ifc_len = len;
        ifc.ifc_buf = buf;
        if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
            if (errno != EINVAL || lastlen != 0) {
                err_sys("ioctl error");
            }
        } else {
            if (ifc.ifc_len == lastlen) {
                break; // success, len has not changed
            }
            lastlen = ifc.ifc_len;
        }
        len += 10 * sizeof(struct ifreq);
        free(buf);
    }

    struct ifi_info *ifihead = NULL;
    struct ifi_info **ifipnext = &ifihead;
    char lastname[IFNAMSIZ];
    lastname[0] = 0;
    char *ptr;
    for (ptr = buf; ptr < buf + ifc.ifc_len;) {
        struct ifreq *ifr = (struct ifreq *)ptr;
        switch (ifr->ifr_addr.sa_family) {
        case AF_INET6:
            len = sizeof(struct sockaddr_in6);
            break;
        case AF_INET:
        default:
            len = sizeof(struct sockaddr_in);
            break;
        }

        ptr += sizeof(ifr->ifr_name) + len;

        if (ifr->ifr_addr.sa_family != family) {
            continue;
        }
        short myflags = 0;
        char *cptr = strchr(ifr->ifr_name, ':');
        if (cptr != NULL) {
            *cptr = 0;
        }
        if (strncmp(lastname, ifr->ifr_name, IFNAMSIZ) == 0) {
            if (doaliases == 0) {
                continue;
            }
            myflags |= IFI_ALIAS;
        }
        memcpy(lastname, ifr->ifr_name, IFNAMSIZ);
        struct ifreq ifrcopy = *ifr;
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy) < 0) {
            err_sys("ioctl error");
        }
        short flags = ifrcopy.ifr_flags;
        if ((flags & IFF_UP) == 0) {
            continue; // ignore if interface not up
        }
        struct ifi_info *ifi = calloc(1, sizeof(struct ifi_info));
        *ifipnext = ifi;
        ifipnext = &ifi->ifi_next;

        ifi->ifi_flags = flags;
        ifi->ifi_myflags = myflags;
        if (ioctl(sockfd, SIOCGIFMTU, &ifrcopy) < 0) {
            err_sys("ioctl error");
        }
        ifi->ifi_mtu = ifrcopy.ifr_mtu;
        memcpy(ifi->ifi_name, ifr->ifr_name, IFI_NAME);
        ifi->ifi_name[IFI_NAME - 1] = '\0';

        struct sockaddr_in *sinptr;
        struct sockaddr_in6 *sin6ptr;
        switch (ifr->ifr_addr.sa_family) {
        case AF_INET:
            sinptr = (struct sockaddr_in *)&ifr->ifr_addr;
            ifi->ifi_addr = calloc(1, sizeof(struct sockaddr_in));
            memcpy(ifi->ifi_addr, sinptr, sizeof(struct sockaddr_in));
            if (flags & IFF_BROADCAST) {
                if (ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy) < 0) {
                    err_sys("ioctl error");
                }
                sinptr = (struct sockaddr_in *)&ifrcopy.ifr_broadaddr;
                ifi->ifi_brdaddr = calloc(1, sizeof(struct sockaddr_in));
                memcpy(ifi->ifi_brdaddr, sinptr, sizeof(struct sockaddr_in));
            }
            if (flags & IFF_POINTOPOINT) {
                if (ioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy) < 0) {
                    err_sys("ioctl error");
                }
                sinptr = (struct sockaddr_in *)&ifrcopy.ifr_broadaddr;
                ifi->ifi_dstaddr = calloc(1, sizeof(struct sockaddr_in));
                memcpy(ifi->ifi_dstaddr, sinptr, sizeof(struct sockaddr_in));
            }
            break;
        case AF_INET6:
            sin6ptr = (struct sockaddr_in6 *)&ifr->ifr_addr;
            ifi->ifi_addr = calloc(1, sizeof(struct sockaddr_in6));
            memcpy(ifi->ifi_addr, sin6ptr, sizeof(struct sockaddr_in6));
            if (flags & IFF_POINTOPOINT) {
                if (ioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy) < 0) {
                    err_sys("ioctl error");
                }
                sin6ptr = (struct sockaddr_in6 *)&ifrcopy.ifr_broadaddr;
                ifi->ifi_dstaddr = calloc(1, sizeof(struct sockaddr_in6));
                memcpy(ifi->ifi_dstaddr, sin6ptr, sizeof(struct sockaddr_in6));
            }
        default:
            break;
        }
    }
    free(buf);
    return ifihead;
}

void free_ifi_info(struct ifi_info *ifihead) {
    struct ifi_info *ifi, *ifinext;
    for (ifi = ifihead; ifi != NULL; ifi = ifinext) {
        if (ifi->ifi_addr != NULL) {
            free(ifi->ifi_addr);
        }
        if (ifi->ifi_brdaddr != NULL) {
            free(ifi->ifi_addr);
        }
        if (ifi->ifi_dstaddr != NULL) {
            free(ifi->ifi_addr);
        }
        ifinext = ifi->ifi_next;
        free(ifi);
    }
}
