#include "unp.h"

int sock_get_port(SA *paddr) {
    if (paddr->sa_family == AF_INET) {
        return ((struct sockaddr_in *)paddr)->sin_port;
    } else if (paddr->sa_family == AF_INET6) {
        return ((struct sockaddr_in6 *)paddr)->sin6_port;
    } else {
        return -1;
    }
}
