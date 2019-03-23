#include "fifo.h"

int main(int argc, char **argv) {
    char fifoname[MAXLINE], buff[MAXLINE];
    // create FIFO with our PID as part of name
    pid_t pid = getpid();
    snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long)pid);
    if (mkfifo(fifoname, FILEMODE) < 0 && errno != EEXIST) {
        err_sys("can't create %s", fifoname);
    }
    // start buffer with pid and a blank
    snprintf(buff, sizeof(buff), "%ld ", (long)pid);
    size_t len = strlen(buff);
    char *ptr = buff + len;
    // fgets() guarantees null byte at end
    if (fgets(ptr, MAXLINE - len, stdin) == NULL) {
        err_sys("fgets error");
    }
    len = strlen(buff);
    // open FIFO to server and write PID and pathname to FIFO
    int writefifo = open(SERV_FIFO, O_WRONLY);
    if (writefifo < 0) {
        err_sys("open error");
    }
    if (write(writefifo, buff, len) < 0) {
        err_sys("write error");
    }
    // now open our FIFO; blocks until server opens for writing
    int readfifo = open(fifoname, O_RDONLY);
    if (readfifo < 0) {
        err_sys("open error");
    }

    ssize_t n;
    while ((n = read(readfifo, buff, MAXLINE)) > 0) {
        if (write(STDOUT_FILENO, buff, n) < 0) {
            err_sys("write error");
        }
    }
    if (n < 0) {
        err_sys("read error");
    }
    close(readfifo);
    unlink(fifoname);
}
