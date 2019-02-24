#include "unp.h"

void dg_cli(FILE *fp, int sockfd, const SA *servaddrptr, socklen_t servlen) {
    char sendline[MAXLINE], recvline[MAXLINE + 1];

    while (fgets(sendline, MAXLINE, fp) != NULL){
        if (sendto(sockfd, sendline, strlen(sendline), 0, servaddrptr, servlen) < 0) {
            err_sys("sendto error");
        }
        if (readable_timeo(sockfd, 5) == 0) {
            fprintf(stderr, "socket timeout\n");
        } else {
            ssize_t n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
            recvline[n] = 0; // null terminate
            if (fputs(recvline, stdout) == EOF) {
                err_sys("fputs error");
            }
        }
    }
}
