#include "unp.h"

ssize_t Write(int fd, const char *buf, size_t n);
ssize_t Send(int fd, const char *buf, size_t n, int flags);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: tcpsend04 <host> <port>");
    }
    int sockfd = tcp_connect(argv[1], argv[2]);
    if (sockfd < 0) {
        err_sys("tcp_connect error");
    }
    Write(sockfd, "123", 3);
    printf("wrote 3 bytes of normal data\n");

    Send(sockfd, "4", 1, MSG_OOB);
    printf("Wrote 1 byte of OOB data\n");

    Write(sockfd, "5", 1);
    printf("wrote 1 byte of normal data\n");
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

