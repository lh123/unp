#include "unp.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>

#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"

#define FILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

void client(int readfd, int writefd), server(int readfd, int writefd);

int main(int argc, char **argv) {
    if (mkfifo(FIFO1, FILEMODE) < 0 && errno != EEXIST) {
        err_sys("can't create %s", FIFO1);
    }
    if (mkfifo(FIFO2, FILEMODE) < 0 && errno != EEXIST) {
        unlink(FIFO1);
        err_sys("can't create %s", FIFO2);
    }
    int readfd, writefd;
    pid_t childpid = fork();
    if (childpid < 0) {
        err_sys("fork error");
    } else if (childpid == 0) {
        if ((readfd = open(FIFO1, O_RDONLY)) < 0) {
            err_sys("open error");
        }
        if ((writefd = open(FIFO2, O_WRONLY)) < 0) {
            err_sys("open error");
        }
        server(readfd, writefd);
        exit(0);
    }
    if ((writefd = open(FIFO1, O_WRONLY)) < 0) {
        err_sys("open error");
    }
    if ((readfd = open(FIFO2, O_RDONLY)) < 0) {
        err_sys("open error");
    }
    client(readfd, writefd);

    if (waitpid(childpid, NULL, 0) < 0) {
        err_sys("waitpid error");
    }
    close(readfd);
    close(writefd);
    unlink(FIFO1);
    unlink(FIFO2);
}
