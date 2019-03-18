#include "unp.h"

#define MAXN 16384 // max # bytes to request from server

int main(int argc, char **argv) {
    if (argc != 6) {
        err_quit("usage: client <hostname or IPaddr> <port> <#children> <#loops/child> <#bytes/request>");
    }
    int nchildren = atoi(argv[3]);
    int nloops = atoi(argv[4]);
    int nbytes = atoi(argv[5]);
    printf("nchildren = %d, nloops = %d, nbytes = %d\n", nchildren, nloops, nbytes);
    char request[MAXLINE];
    snprintf(request, sizeof(request), "%d\n", nbytes);

    int i;
    for (i = 0; i < nchildren; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            err_sys("fork error");
        } else if (pid == 0) {
            int j;
            char reply[MAXN];
            for (j = 0; j < nloops; j++) {
                int fd = tcp_connect(argv[1], argv[2]);
                if (fd < 0) {
                    err_sys("tcp_connect error");
                }
                if (write(fd, request, strlen(request)) < 0) {
                    err_sys("write error");
                }
                ssize_t n = readn(fd, reply, nbytes);
                if (n != nbytes) {
                    err_quit("server returned %ld bytes", n);
                }
                close(fd);
            }
            printf("child %d done\n", i);
            exit(0);
        }
    }

    while (wait(NULL) > 0)
        ;
    if (errno != ECHILD) {
        err_sys("wait error");
    }
}
