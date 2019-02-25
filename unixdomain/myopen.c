#include "unp.h"

int my_open(const char *pathname, int mode) {
    int sockfd[2];
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd) < 0) {
        err_sys("socketpair error");
    }

    pid_t childpid;
    if ((childpid = fork()) < 0) {
        err_sys("fork error");
    } else if (childpid == 0) {
        close(sockfd[0]);
        char argsockfd[10], argmode[10];
        snprintf(argsockfd, sizeof(argsockfd), "%d", sockfd[1]);
        snprintf(argmode, sizeof(argmode), "%d", mode);
        execl("./openfile.out", "openfile", argsockfd, pathname, argmode, NULL);
        err_sys("execl error");
    }
    close(sockfd[1]);
    int status;
    if (waitpid(childpid, &status, 0) < 0) {
        err_sys("waitpid error");
    }

    if (WIFEXITED(status) == 0) {
        err_quit("child did not terminated");
    }

    int fd;
    char c;
    if ((status = WEXITSTATUS(status)) == 0) {
        if (read_fd(sockfd[0], &c, 1, &fd) < 0) {
            err_sys("read_fd error");
        }
    } else {
        errno = status;
        fd = -1;
    }
    close(sockfd[0]);
    return fd;
}
