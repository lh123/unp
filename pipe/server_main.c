#include "fifo.h"

void server(int readfd, int writefd);

int main(int argc, char **argv) {
    if (mkfifo(FIFO1, FILEMODE) < 0 && errno != EEXIST) {
        err_sys("can't create %s", FIFO1);
    }
    if (mkfifo(FIFO2, FILEMODE) < 0 && errno != EEXIST) {
        unlink(FIFO1);
        err_sys("can't create %s", FIFO2);
    }
    int readfd = open(FIFO1, O_RDONLY);
    int writefd = open(FIFO2, O_WRONLY);
    if (readfd < 0 || writefd < 0) {
        err_sys("open error");
    }
    server(readfd, writefd);
}
