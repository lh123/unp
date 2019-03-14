#include "unp.h"
#include <netdb.h>

void dg_cli(FILE *fp, int sockfd, struct sockaddr *addr, socklen_t socklen) {
    char sendline[MAXLINE], recvline[MAXLINE + 1];
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, addr, socklen) < 0) {
            err_sys("sendto error");
        }
        ssize_t n = recv(sockfd, recvline, MAXLINE, 0);
        if (n < 0) {
            err_sys("recv error");
        }
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
    if (ferror(fp)) {
        err_sys("fgets error");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        err_quit("usage: udpcli01 [ <hostname> ...] <hostname>");
    }
    struct addrinfo *ai;
    u_char *ptr = NULL;
    int len;
    if (argc > 2) {
        len = inet6_rth_space(IPV6_RTHDR_TYPE_0, argc - 2);
        ptr = malloc(len);
        inet6_rth_init(ptr, len, IPV6_RTHDR_TYPE_0, argc - 2);
        int i;
        for (i = 1; i < argc - 1; i++) {
            ai = host_serv(argv[i], NULL, AF_INET6, 0);
            if (ai == NULL) {
                err_sys("host_serv error");
            }
            inet6_rth_add(ptr, &((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr);
        }
    }
    ai = host_serv(argv[argc - 1], SERV_PORT_STR, AF_INET6, SOCK_DGRAM);
    int sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sockfd < 0) {
        err_sys("socket error");
    }
    if (ptr) {
        if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_RTHDR, ptr, len) < 0) {
            err_sys("setsockopt error");
        }
        free(ptr);
    }
    dg_cli(stdin, sockfd, ai->ai_addr, ai->ai_addrlen);
}
