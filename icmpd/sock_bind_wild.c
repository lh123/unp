#include "unp.h"

int sock_bind_wild(int sockfd, int family) {
    if (family == AF_INET) {
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = htonl(INADDR_ANY);
        sin.sin_port = htons(0);

        if (bind(sockfd, (SA *)&sin, sizeof(sin)) < 0) {
            err_sys("sock_bind_wild error");
        }
        socklen_t len = sizeof(sin);
        if (getsockname(sockfd, (SA *)&sin, &len) < 0) {
            err_sys("sock_bind_wild error");
        }
        return sin.sin_port;
    } else if (family == AF_INET6) {
        struct sockaddr_in6 sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin6_family = AF_INET6;
        memcpy(&sin.sin6_addr, &in6addr_any, sizeof(in6addr_any));
        sin.sin6_port = htons(0);

        if (bind(sockfd, (SA *)&sin, sizeof(sin)) < 0) {
            err_sys("sock_bind_wild error");
        }
        socklen_t len = sizeof(sin);
        if (getsockname(sockfd, (SA *)&sin, &len) < 0) {
            err_sys("sock_bind_wild error");
        }
        return sin.sin6_port;
    }
    return -1;
}
