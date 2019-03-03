#include "unp.h"
#include <netdb.h>

void dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: udpcli <hostname/IPaddress> <service/port>");
    }
    struct addrinfo hint, *res, *aiptr;
    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_DGRAM;

    int err = getaddrinfo(argv[1], argv[2], &hint, &res);
    if (err != 0) {
        err_quit("getaddrinfo error for %s, %s: %s", argv[1], argv[2], gai_strerror(err));
    }
    int sockfd;
    for (aiptr = res; aiptr != NULL; aiptr = aiptr->ai_next) {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd >= 0) {
            break;
        }
    }
    if (aiptr == NULL) {
        err_sys("socket error for %s, %s", argv[1], argv[2]);
    }
    freeaddrinfo(res);
    
    printf("dst: %s\n", sock_ntop(aiptr->ai_addr, aiptr->ai_addrlen));
    dg_cli(stdin, sockfd, aiptr->ai_addr, aiptr->ai_addrlen);
}
