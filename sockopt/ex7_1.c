#include "unp.h"

int main(void) {
    int tcpfd = socket(AF_INET, SOCK_STREAM, 0);
    int udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (tcpfd < 0 || udpfd < 0) {
        err_sys("socket error");
    }
    int snd, rcv;
    socklen_t len = sizeof(int);
    if (getsockopt(tcpfd, SOL_SOCKET, SO_SNDBUF, &snd, &len) == -1) {
        err_sys("getsockopt error");
    }
    len = sizeof(int);
    if (getsockopt(tcpfd, SOL_SOCKET, SO_RCVBUF, &rcv, &len) == -1) {
        err_sys("getsockopt error");
    }
    printf("tcp buffer send: %d recv: %d\n", snd, rcv);

    len = sizeof(int);
    if (getsockopt(udpfd, SOL_SOCKET, SO_SNDBUF, &snd, &len) == -1) {
        err_sys("getsockopt error");
    }
    len = sizeof(int);
    if (getsockopt(udpfd, SOL_SOCKET, SO_RCVBUF, &rcv, &len) == -1) {
        err_sys("getsockopt error");
    }
    printf("udp buffer send: %d recv: %d\n", snd, rcv);
    close(tcpfd);
    close(udpfd);
}
