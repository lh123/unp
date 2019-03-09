#include "unp.h"

ssize_t Write(int fd, const char *buf, size_t n);
ssize_t Send(int fd, const char *buf, size_t n, int flags);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: tcpsend05 <host> <port>");
    }
    int sockfd = tcp_connect(argv[1], argv[2]);
    const int size = 32768;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) < 0) {
        err_sys("setsockopt error");
    }
    char buff[16384];
    Write(sockfd, buff, 16384);
    printf("wrote 16384 bytes of normal data\n");
    sleep(5);

    Send(sockfd, "a", 1, MSG_OOB);
    printf("wrote 1 byte of OOB data\n");

    Write(sockfd, buff, 1024);
    printf("wrote 1024 bytes of normal data\n");
}

ssize_t Write(int fd, const char *buf, size_t n) {
    ssize_t nw;
    if ((nw = writen(fd, buf, n)) < 0) {
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
