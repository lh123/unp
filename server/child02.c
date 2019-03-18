#include "unp.h"

extern long *cptr;

void child_main(int i, int listenfd);
void web_child(int sockfd);

pid_t child_make(int i, int listenfd) {
    pid_t pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        child_main(i, listenfd);
        exit(0);
    }
    return pid;
}

void child_main(int i, int listenfd) {
    printf("child %ld starting\n", (long)getpid());
    struct sockaddr_storage cliaddr;
    for (;;) {
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
        if (connfd < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                err_sys("accept error");
            }
        }
        web_child(connfd);
        close(connfd);
        cptr[i]++;
    }
}
