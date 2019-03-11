#include "unp.h"
#include <pthread.h>

void *copyto(void *arg);

static int sockfd;
static FILE *fp;
static int stdin_eof;

ssize_t readline_r(int fd, void *vptr, size_t maxlen);

void str_cli(FILE *fp_arg, int sockfd_arg) {
    fp = fp_arg;
    sockfd = sockfd_arg;
    pthread_t tid;
    int err;
    if ((err = pthread_create(&tid, NULL, copyto, NULL)) != 0) {
        err_exit(err, "pthread_create error");
    }
    char recvline[MAXLINE];
    int n;
    while ((n = readline_r(sockfd, recvline, MAXLINE)) > 0) {
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
    if (n < 0) {
        err_sys("readline error");
    }
    if (stdin_eof == 0) {
        err_quit("server terminated prematurely");
    }
}

void *copyto(void *arg) {
    char sendline[MAXLINE];
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (writen(sockfd, sendline, strlen(sendline)) < 0) {
            err_sys("writen error");
        }
    }
    if (ferror(fp)) {
        err_sys("fgets error");
    }
    stdin_eof = 1;
    shutdown(sockfd, SHUT_WR);
    return NULL;
}
