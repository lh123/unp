#include "unp.h"
#include <errno.h>
#include <string.h>

char *sock_ntop(const struct sockaddr *sa, socklen_t addrlen) {
    char portstr[8];
    static char str[128];

    switch (sa->sa_family) {
    case AF_INET: {
        struct sockaddr_in *sin = (struct sockaddr_in *)sa;
        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) {
            return NULL;
        }
        if (ntohs(sin->sin_port) != 0) {
            snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
            strncat(str, portstr, sizeof(str) - strlen(str) - 1);
        }
        return str;
    }
    case AF_INET6: {
        struct sockaddr_in6 *sin = (struct sockaddr_in6 *)sa;
        if (inet_ntop(AF_INET6, &sin->sin6_addr, str, sizeof(str)) == NULL) {
            return NULL;
        }
        if (ntohs(sin->sin6_port) != 0) {
            snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin6_port));
            strncat(str, portstr, sizeof(str) - strlen(str) - 1);
        }
        return str;
    }
    }
    return NULL;
}

ssize_t readn(int fd, void *vptr, size_t n) {
    size_t nleft = n;
    char *ptr = vptr;
    while (nleft > 0) {
        ssize_t nread = read(fd, ptr, nleft);
        if (nread < 0) {
            if (errno == EINTR) {
                nread = 0;
            } else {
                return -1;
            }
        } else if (nread == 0) {
            break;
        }
        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
}

ssize_t writen(int fd, const void *vptr, size_t n) {
    size_t nleft = n;
    const char *ptr = vptr;
    while (nleft > 0) {
        ssize_t nwritten = write(fd, ptr, nleft);
        if (nwritten < 0) {
            if (errno = EINTR) {
                nwritten = 0;
            } else {
                return -1;
            }
        } else if (nwritten == 0) {
            return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

ssize_t readline(int fd, void *vptr, size_t maxlen) {
    int n = 0;
    char *ptr = vptr;
    while (n < maxlen - 1) {
        ssize_t rc = read(fd, ptr, 1);
        if (rc == 1) {
            if (*ptr++ == '\n') {
                break;
            }
            n++;
        } else if (rc == 0) {
            break;
        } else {
            if (errno != EINTR) {
                return -1;
            }
        }
    }
    *ptr = 0;
    return n + 1;
}