#include "unp.h"
#include <sys/un.h>

static void str_cli(FILE *fp, int sockfd);
ssize_t write_cred(int fd, void *ptr, size_t nbyte);

int main(int argc, char **argv) {
    int sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sockfd < 0) {
        err_sys("socket error");
    }
    struct sockaddr_un servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, UNIXSTR_PATH);

    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("connect error");
    }

    str_cli(stdin, sockfd);
}

static void str_cli(FILE *fp, int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE + 1];
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (write_cred(sockfd, sendline, strlen(sendline)) < 0) {
            err_sys("write error");
        }

        ssize_t n;
        if ((n = readline(sockfd, recvline, MAXLINE)) < 0) {
            err_sys("readline error");
        }
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
}
