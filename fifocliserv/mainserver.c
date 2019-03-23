#include "fifo.h"

int main(int argc, char **argv) {
    if (mkfifo(SERV_FIFO, FILEMODE) < 0 && errno != EEXIST) {
        err_sys("can't create %s", SERV_FIFO);
    }
    int readfifo = open(SERV_FIFO, O_RDONLY);
    int dummyfd = open(SERV_FIFO, O_WRONLY); // never used
    if (readfifo < 0 || dummyfd < 0) {
        err_sys("open error");
    }

    ssize_t n;
    char buff[MAXLINE + 1], fifoname[MAXLINE];
    while ((n = readline(readfifo, buff, MAXLINE)) > 0) {
        if (buff[n - 1] == '\n') {
            n--;
        }
        buff[n] = '\0';
        char *ptr = strchr(buff, ' ');
        if (ptr == NULL) {
            err_msg("bogus request: %s", buff);
            continue;
        }
        *ptr++ = 0; // null terminate PID, ptr = pathname
        pid_t pid = atol(buff);
        snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long)pid);
        int writefifo= open(fifoname, O_WRONLY);
        if (writefifo < 0) {
            err_msg("cannot open: %s", fifoname);
            continue;
        }
        int fd = open(ptr, O_RDONLY);
        if (fd < 0) {
            snprintf(buff + n, sizeof(buff) - n, ": can't open, %s\n", strerror(errno));
            n = strlen(ptr);
            if (write(writefifo, ptr, n) < 0) {
                err_sys("write error");
            }
            close(writefifo);
        } else {
            while ((n = read(fd, buff, MAXLINE)) > 0) {
                if (write(writefifo, buff, n) < 0) {
                    err_sys("write error");
                }
            }
            if (n < 0) {
                err_sys("read error");
            }
            close(fd);
            close(writefifo);
        }
    }
    if (n < 0) {
        err_sys("readline error");
    }
}