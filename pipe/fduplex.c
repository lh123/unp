#include "unp.h"

int main(int argc, char **argv) {
    int fd[2];
    if (pipe(fd) < 0) {
        err_sys("pipe error");
    }
    int n;
    char c;
    pid_t childpid = fork();
    if (childpid < 0) {
        err_sys("fork error");
    } else if (childpid == 0) {
        sleep(3);
        if ((n = read(fd[0], &c, 1)) != 1) {
            err_quit("child: read returned %d", n);
        }
        printf("child read %c\n", c);
        if (write(fd[0], "c", 1) < 0) {
            err_sys("write error");
        }
        exit(0);
    }

    if (write(fd[1], "p", 1) < 0) {
        err_sys("write error");
    }
    if ((n = read(fd[1], &c, 1)) != 1) {
        err_quit("parent: read returned %d", n);
    }
    printf("parent read %c\n", c);
}
