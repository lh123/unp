#include "unp.h"
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

const char *inet_ntop(int family, const void *addptr, char *strptr, size_t len) {
    const uint8_t *p = (const uint8_t *)addptr;
    if (family == AF_INET) {
        char temp[INET_ADDRSTRLEN];
        snprintf(temp, sizeof(temp), "%u.%u.%u.%u", p[0], p[1], p[2], p[3]);
        if (strlen(temp) >= len) {
            errno = ENOSPC;
            return NULL;
        }
        strcpy(strptr, temp);
        return strptr;
    }
    errno = EAFNOSUPPORT;
    return NULL;
}
