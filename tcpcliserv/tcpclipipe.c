#include "unp.h"

static void str_cli_pipe(FILE *fp, int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE];
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (writen(sockfd, sendline, 1) < 0) {
            err_sys("write error");
        }
        if (writen(sockfd, sendline + 1, strlen(sendline) - 1) < 0) {
            err_sys("write error");
        }
        if (readline(sockfd, recvline, MAXLINE) <= 0) {
            err_quit("str_cli: server terminated prematurely");
        }
        fputs(recvline, stdout);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: tcpcli <IPaddress>");
    }
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGPIPE, &act, NULL) < 0) {
        err_sys("sigaction SIGPIPE error");
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        err_sys("socket error");
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        err_sys("inet_pton error for %s", argv[1]);
    }
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("connect error");
    }
    str_cli_pipe(stdin, sockfd);
}