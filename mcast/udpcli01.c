#include "unp.h"
#include <netdb.h>

void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: udpcli01 <hostname/IPaddress> <service/port>");
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
        sockfd = socket(aiptr->ai_family, aiptr->ai_socktype, aiptr->ai_protocol);
        if (sockfd >= 0) {
            break;
        }
    }
    if (aiptr == NULL) {
        err_quit("getaddrinfo error for %s, %s", argv[1], argv[2]);
    }

    dg_cli(stdin, sockfd, aiptr->ai_addr, aiptr->ai_addrlen);
    freeaddrinfo(res);
}

void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen) {
    char sendline[MAXLINE], recvline[MAXLINE + 1];
    struct sockaddr_storage reply_addr;
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen) < 0) {
            err_sys("sendto error");
        }
        socklen_t len = servlen;
        ssize_t n = recvfrom(sockfd, recvline, MAXLINE, 0, (SA *)&reply_addr, &len);
        if (n < 0) {
            err_sys("recvfrom error");
        }
        recvline[n] = 0;
        printf("from %s: %s", sock_ntop((SA *)&reply_addr, len), recvline);
    }
    if (ferror(fp)) {
        err_sys("fgets error");
    }
}
