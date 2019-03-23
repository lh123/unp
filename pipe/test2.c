#include "unp.h"

static void sig_pipe(int signo);
static void sig_int(int signo);

int main(int argc, char **argv) {
    int fd[2];
    if (pipe(fd) < 0) {
        err_sys("pipe error");
    }

    struct sigaction act;
    act.sa_handler = sig_pipe;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGPIPE, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    act.sa_handler = sig_int;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        err_sys("sigaction error");
    }

    pid_t childpid = fork();
    if (childpid < 0) {
        err_sys("fork error");
    } else if (childpid == 0) {
        printf("child closing pipe read descriptor\n");
        close(fd[0]);
        sleep(50);
    }

    close(fd[0]);
    sleep(3);
    fd_set wset;
    FD_ZERO(&wset);
    FD_SET(fd[1], &wset);
    int n = select(fd[1] + 1, NULL, &wset, NULL, NULL);
    printf("select returned %d\n", n);
    if (FD_ISSET(fd[1], &wset)) {
        printf("fd[1] writable\n");
        if (write(fd[1], "hello", 5) < 0) {
            err_sys("write error");
        }
    }
}

static void sig_pipe(int signo) {
    printf("\nSIGPIPE\n");
}

static void sig_int(int signo) {
    printf("\nSIGINT\n");
}
