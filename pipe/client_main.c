#include "fifo.h"

void client(int readfd, int writefd);

int main(int argc, char **argv) {
    int writefd = open(FIFO1, O_WRONLY);
    int readfd = open(FIFO2, O_RDONLY);
    if (writefd < 0 || readfd < 0) {
        err_sys("open error");
    }
    client(readfd, writefd);

    close(readfd);
    close(writefd);
    unlink(FIFO1);
    unlink(FIFO2);
}
