#include "unp.h"

void recv_all(int recvfd, socklen_t salen);
void send_all(int sendfd, SA *sadest, socklen_t salen);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: sendrecv <IP-multicast-address> <port#>");
    }
    struct sockaddr_storage sssend, ssrecv;
    socklen_t sslen = sizeof(sssend);
    int sendfd = udp_client(argv[1], argv[2], (SA *)&sssend, &sslen);
    if (sendfd < 0) {
        err_sys("sendfd error");
    }
    int recvfd = socket(sssend.ss_family, SOCK_DGRAM, 0);

    const int on = 1;
    if (setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }
    memcpy(&ssrecv, &sssend, sslen);
    if (bind(recvfd, (SA *)&sssend, sslen) < 0) {
        err_sys("bind error");
    }
    if (mcast_join(recvfd, (SA *)&sssend, sslen, NULL, 0) < 0) {
        err_sys("mcast_join error");
    }
    if (mcast_set_loop(sendfd, 0) < 0) {
        err_sys("mcast_set_loop error");
    }
    pid_t pid;
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        recv_all(recvfd, sslen);
    }
    send_all(sendfd, (SA *)&sssend, sslen);
}
