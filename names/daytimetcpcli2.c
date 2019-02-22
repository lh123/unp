#include "unp.h"
#include <netdb.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: daytimetcpcli2 <hostname> <service>");
    }
    struct sockaddr_in servaddr;
    struct in_addr addr, *addrs[2], **addrpptr;
    struct hostent *hptr;
    struct servent *sptr;
    int sockfd;
    char recvline[MAXLINE + 1];

    if (inet_pton(AF_INET, argv[1], &addr) == 1) {
        addrs[0] = &addr;
        addrs[1] = NULL;
        addrpptr = addrs;
    } else if ((hptr = gethostbyname(argv[1])) != NULL) {
        addrpptr = (struct in_addr **)hptr->h_addr_list;
    } else {
        err_quit("hostname error for %s: %s", argv[1], hstrerror(h_errno));
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    int n = atoi(argv[2]);
    if (n > 0) {
        servaddr.sin_port = htons(n);
    } else if ((sptr = getservbyname(argv[2], "tcp")) != NULL) {
        servaddr.sin_port = sptr->s_port;
    } else {
        err_quit("getservbyname error for %s", argv[2]);
    }
    for (; *addrpptr != NULL; addrpptr++) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        memcpy(&servaddr.sin_addr, *addrpptr, sizeof(struct in_addr));
        printf("trying %s\n", sock_ntop((struct sockaddr *)&servaddr, sizeof(servaddr)));

        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0) {
            break;
        }
        err_ret("connect error");
        close(sockfd);
    }

    if (*addrpptr == NULL) {
        err_quit("unable to connect");
    }

    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
}
