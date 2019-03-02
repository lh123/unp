#include "unpifi.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: prifinfo <inet4|inet6> <doaliases>");
    }
    int family, doaliases;
    if (strcmp(argv[1], "inet4") == 0) {
        family = AF_INET;
    } else if (strcmp(argv[1], "inet6") == 0) {
        family = AF_INET6;
    } else {
        err_quit("invalid <address-family>");
    }
    doaliases = atoi(argv[2]);

    struct ifi_info *ifihead, *ifi;
    ifihead = ifi = get_ifi_info(family, doaliases);
    for (; ifi != NULL; ifi = ifi->ifi_next) {
        printf("%s: ", ifi->ifi_name);
        printf("<");
        if (ifi->ifi_flags & IFF_UP) printf("UP ");
        if (ifi->ifi_flags & IFF_BROADCAST) printf("BCAST ");
        if (ifi->ifi_flags & IFF_MULTICAST) printf("MCAST ");
        if (ifi->ifi_flags & IFF_LOOPBACK) printf("LOOP ");
        if (ifi->ifi_flags & IFF_POINTOPOINT) printf("P2P ");
        printf(">\n");
        if (ifi->ifi_mtu != 0) {
            printf("  MTU: %d\n", ifi->ifi_mtu);
        }

        struct sockaddr *sa = ifi->ifi_addr;
        if (sa != NULL) {
            printf("  IP addr: %s\n", sock_ntop(sa, sizeof(*sa)));
        }
        if ((sa = ifi->ifi_brdaddr) != NULL) {
            printf("  broadcast addr: %s\n", sock_ntop(sa, sizeof(*sa)));
        }
        if ((sa = ifi->ifi_dstaddr) != NULL) {
            printf("  destination addr: %s\n", sock_ntop(sa, sizeof(*sa)));
        }
    }
    free_ifi_info(ifihead);
}
