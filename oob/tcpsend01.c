#include "unp.h"

ssize_t Write(int fd, const char *buf, size_t n);
ssize_t Send(int fd, const char *buf, size_t n, int flags);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: tcpsend01 <host> <port>");
    }
    int sockfd = tcp_connect(argv[1], argv[2]);
    Write(sockfd, "123", 3);
    printf("wrote 3 bytes of normal data\n");
    sleep(1);

    Send(sockfd, "4", 1, MSG_OOB);
    printf("wrote 1 byte of OOB data\n");
    sleep(1);

    Write(sockfd, "56", 2);
    printf("wrote 2 bytes of normal data\n");
    sleep(1);

    Send(sockfd, "7", 1, MSG_OOB);
    printf("wrote 1 byte of OOB data\n");
    sleep(1);

    Write(sockfd, "89", 2);
    printf("wrote 2 bytes of normal data\n");
    sleep(1);
}

ssize_t Write(int fd, const char *buf, size_t n) {
    ssize_t nw;
    if ((nw = write(fd, buf, n)) < 0) {
        err_sys("write error");
    }
    return nw;
}

ssize_t Send(int fd, const char *buf, size_t n, int flags) {
    ssize_t nw;
    if ((nw = send(fd, buf, n, flags)) < 0) {
        err_sys("send error");
    }
    return nw;
}
