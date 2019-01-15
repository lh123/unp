#include "unp.h"
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int inet_pton(int family, const char *strptr, void *addrptr) {
    if (family == AF_INET) {
        struct in_addr in_val;
        if ((in_val.s_addr = inet_addr(strptr)) != INADDR_NONE) {
            memcpy(addrptr, &in_val, sizeof(struct in_addr));
            return 1;
        }
        return 0;
    }
    errno = EAFNOSUPPORT;
    return -1;
}
