#include "unp.h"
#include <netdb.h>

struct addrinfo *host_serv(const char *hostname, const char *service, int family, int socktype) {
    struct addrinfo hint, *res;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = family;
    hint.ai_socktype = socktype;
    hint.ai_flags = AI_CANONNAME;

    int n = getaddrinfo(hostname, service, &hint, &res);
    if (n != 0) {
        return NULL;
    }
    return res;
}
