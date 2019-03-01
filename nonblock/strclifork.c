#include "unp.h"

void str_cli(FILE *fp, int sockfd) {
    pid_t pid;
    char sendline[MAXLINE], recvline[MAXLINE];

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        while (readline(sockfd, recvline, MAXLINE) > 0) {
            if (fputs(recvline, stdout) == EOF) {
                err_sys("fputs error");
            }
        }
        kill(getppid(), SIGTERM);
        exit(0);
    }

    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (writen(sockfd, sendline, strlen(sendline)) < 0) {
            err_sys("writen error");
        }
    }
    shutdown(sockfd, SHUT_WR);
    pause();
}
