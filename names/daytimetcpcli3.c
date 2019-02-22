#include "unp.h"
#include <netdb.h>

int main(int argc, char **argv) {
    struct hostent *hp;
    struct servent *sp;

    if (argc != 3) {
        err_quit("usage: daytimetcpcli3 <hostname> <service>");
    }

    if ((hp = gethostbyname(argv[1])) == NULL) {
        err_quit("hostname error for %s: %s", argv[1], hstrerror(h_errno));
    }
    if ((sp = getservbyname(argv[2], "tcp")) == NULL) {
        err_quit("getservbyname error for %s: %s", argv[2], hstrerror(h_errno));
    }

    struct in_addr **pptr = (struct in_addr **)hp->h_addr_list;
    int sockfd;
    struct sockaddr *sa;
    struct sockaddr_in servaddr;
    struct sockaddr_in6 servaddr6;
    socklen_t salen;
    char recvline[MAXLINE + 1];

    for (; *pptr != NULL; pptr++) {
        sockfd = socket(hp->h_addrtype, SOCK_STREAM, 0);

        if (hp->h_addrtype == AF_INET) {
            sa = (struct sockaddr *)&servaddr;
            salen = sizeof(servaddr);
        } else if (hp->h_addrtype == AF_INET6) {
            sa = (struct sockaddr *)&servaddr6;
            salen = sizeof(servaddr6);
        } else {
            err_quit("unknown addrtype %d", hp->h_addrtype);
        }

        memset(sa, 0, salen);
        sa->sa_family = hp->h_addrtype;
        if (sa->sa_family == AF_INET) {
            servaddr.sin_port = sp->s_port;
            memcpy(&servaddr.sin_addr, *pptr, sizeof(struct in_addr));
        } else if (sa->sa_family == AF_INET6) {
            servaddr6.sin6_port = sp->s_port;
            memcpy(&servaddr6.sin6_addr, *pptr, sizeof(struct in6_addr));
        }
        printf("trying %s\n", sock_ntop(sa, salen));

        if (connect(sockfd, sa, salen) == 0) {
            break;
        }
        err_ret("connect error");
        close(sockfd);
    }

    if (*pptr == NULL) {
        err_quit("unable to connect");
    }

    int n;
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
}
