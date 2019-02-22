#include "unp.h"
#include <netdb.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: daytimetcpcli1 <hostname> <service>");
    }
    struct in_addr inetaddr;
    struct in_addr *inetaddrp[2];
    struct in_addr **pptr;
    struct hostent *hp = gethostbyname(argv[1]);

    int ret;
    if (hp == NULL) {
        if ((ret = inet_pton(AF_INET, argv[1], &inetaddr)) != 1) {
            err_quit("hostname error for %s: %s", argv[1], hstrerror(h_errno));
        } else {
            inetaddrp[0] = &inetaddr;
            inetaddrp[1] = NULL;
            pptr = inetaddrp;
        }
    } else {
        pptr = (struct in_addr **)hp->h_addr_list;
    }
    struct servent *sp = getservbyname(argv[2], "tcp");
    if (sp == NULL) {
        err_quit("getservbyname error for %s", argv[2]);
    }

    int sockfd;
    struct sockaddr_in servaddr;
    for (; *pptr != NULL; pptr++) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = sp->s_port;
        memcpy(&servaddr.sin_addr, *pptr, sizeof(struct in_addr));
        printf("trying %s\n", sock_ntop((struct sockaddr *)&servaddr, sizeof(servaddr)));

        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0) {
            break;
        }
        err_ret("connect error");
        close(sockfd);
    }
    if (*pptr == NULL) {
        err_quit("unable to connect");
    }

    int n;
    char recvline[MAXLINE + 1];
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
}
