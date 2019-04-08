#include "unp.h"
#include <sys/msg.h>
#include <sys/mman.h>

#define MAXMSG  (8192 + sizeof(long))

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: svmsgread <pathname>");
    }
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {
        err_sys("pipe error");
    }
    struct msgbuf *buff;
    if ((buff = mmap(NULL, MAXMSG, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0)) == MAP_FAILED) {
        err_sys("mmap error");
    }
    pid_t childpid;
    ssize_t nread, n;
    char c;
    if ((childpid = fork()) < 0) {
        err_sys("fork error");
    } else if (childpid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        int mqid;
        key_t key;
        if ((key = ftok(argv[1], 0)) < 0) {
            err_sys("ftok error");
        }
        if ((mqid = msgget(key, 0)) < 0) {
            err_sys("msgget error");
        }
        for (;;) {
            if ((nread = msgrcv(mqid, buff, MAXMSG, 0, 0)) < 0) {
                err_sys("msgrcv error");
            }
            if (write(pipe2[1], &nread, sizeof(ssize_t)) < 0) {
                err_sys("write error");
            }
            if ((n = read(pipe1[0], &c, 1)) != 1) {
                err_quit("child: read on pipe returned %d", (int)n);
            }
        }
        exit(0);
    }
    close(pipe1[0]);
    close(pipe2[1]);

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(pipe2[0], &rset);
    for (;;) {
        if ((n = select(pipe2[0] + 1, &rset, NULL, NULL, NULL)) != 1) {
            err_sys("select returned %d", (int)n);
        }
        if (FD_ISSET(pipe2[0], &rset)) {
            if ((n = read(pipe2[0], &nread, sizeof(ssize_t))) != sizeof(ssize_t)) {
                err_sys("read error");
            }
            printf("read %d bytes, type = %ld\n", (int)nread, buff->mtype);
            if (write(pipe1[1], &c, 1) != 1) {
                err_sys("write error");
            }
        } else {
            err_quit("pipe2[0] not ready");
        }
    }
    kill(childpid, SIGTERM);
}
