#include "unp.h"
#include <sys/un.h>

void dg_cli(FILE *fp, int sockfd, const SA *servaddr, socklen_t serlen);

int main(int argc, char **argv) {
    int sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        err_sys("socket error");
    }
    struct sockaddr_un cliaddr, servaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sun_family = AF_LOCAL;
    strcpy(cliaddr.sun_path, tmpnam(NULL));

    printf("cliaddr: %s\n", cliaddr.sun_path);

    if (bind(sockfd, (SA *)&cliaddr, sizeof(cliaddr)) < 0) {
        err_sys("bind error");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, UNIXDG_PATH);

    dg_cli(stdin, sockfd, (SA *)&servaddr, sizeof(servaddr));
}

void dg_cli(FILE *fp, int sockfd, const SA *servaddr, socklen_t serlen) {
    char sendline[MAXLINE], recvline[MAXLINE + 1];
    
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, servaddr, serlen) < 0) {
            err_sys("sendto error");
        }

        ssize_t n;
        if ((n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL)) < 0) {
            err_sys("recvfrom error");
        }
        
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
}
