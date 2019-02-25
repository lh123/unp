#include "unp.h"

#define PORT 9999
#define ADDR "127.0.0.1"
#define MAXBACKLOG 100

struct sockaddr_in serv;
pid_t pid;

int pipefd[2];
#define pfd pipefd[1]
#define cfd pipefd[0]

void do_parent(void);
void do_child(void);

int main(int argc, char **argv) {
    if (argc != 1) {
        err_quit("usage: backlog");
    }
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd) < 0) {
        err_sys("socketpair error");
    }
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    if (inet_pton(AF_INET, ADDR, &serv.sin_addr) != 1) {
        err_sys("inet_pton error");
    }
    
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        do_child();
    } else {
        do_parent();
    }
}

void parent_alrm(int signo) {

}

void do_parent(void) {
    close(cfd);
    struct sigaction act;
    act.sa_handler = parent_alrm;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGALRM, &act, NULL) < 0) {
        err_sys("sigaction error");
    }

    int backlog, junk, fd[MAXBACKLOG + 1];
    for (backlog = 0; backlog <= 14; backlog++) {
        printf("backlog = %d: ", backlog);
        if (write(pfd, &backlog, sizeof(int)) < 0) {
            err_sys("write error");
        }
        if (read(pfd, &junk, sizeof(int)) < 0) {
            err_sys("read error");
        }

        int j;
        for (j = 1; j <= MAXBACKLOG; j++) {
            fd[j] = socket(AF_INET, SOCK_STREAM, 0);
            if (fd[j] < 0) {
                err_sys("socket error");
            }
            alarm(2);
            if (connect(fd[j], (SA *)&serv, sizeof(serv)) < 0) {
                if (errno != EINTR) {
                    err_sys("connect error, j = %d", j);
                }
                printf("timeout, %d connection completed\n", j - 1);
                int k;
                for (k = 1; k <= j; k++) {
                    close(fd[k]);
                }
                break;
            }
            alarm(0);
        }
        if (j > MAXBACKLOG) {
            printf("%d connection?\n", MAXBACKLOG);
            int k;
            for (k = 1; k < j; k++) {
                close(fd[k]);
            }
            break;
        }
    }
    backlog = -1;
    if (write(pfd, &backlog, sizeof(int)) < 0) {
        err_sys("write error");
    }
}

void do_child(void) {
    close(pfd);
    int listenfd, backlog, junk;
    const int on = 1;

    if (read(cfd, &backlog, sizeof(int)) < 0) {
        err_sys("read error");
    }
    while (backlog >= 0) {
        listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
            err_sys("setsockopt error");
        }
        if (bind(listenfd, (SA *)&serv, sizeof(serv)) < 0) {
            err_sys("bind error");
        }
        if (listen(listenfd, backlog) < 0) {
            err_sys("listen error");
        }

        if (write(cfd, &junk, sizeof(int)) < 0) {
            err_sys("write error");
        }

        if (read(cfd, &backlog, sizeof(int)) < 0) {
            err_sys("read error");
        }
        close(listenfd);
    }
}
