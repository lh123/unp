#include "unp.h"

char *sock_ntop_host(struct sockaddr *addr, socklen_t socklen) {
    static char ipstr[INET6_ADDRSTRLEN];
    if (addr->sa_family == AF_INET) {
        if (inet_ntop(AF_INET, &((struct sockaddr_in *)addr)->sin_addr, ipstr, sizeof(ipstr)) != NULL) {
            return ipstr;
        }
    } else if (addr->sa_family == AF_INET6) {
        if (inet_ntop(AF_INET6, &((struct sockaddr_in6 *)addr)->sin6_addr, ipstr, sizeof(ipstr)) != NULL) {
            return ipstr;
        }
    }
    return NULL;
}
