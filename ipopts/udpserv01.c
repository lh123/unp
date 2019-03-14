#include "unp.h"

void dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen);

int main(int argc, char **argv) {
    int sockfd;
    if (argc == 1) {
        sockfd = udp_server(NULL, SERV_PORT_STR, NULL);
    } else if (argc == 2) {
        sockfd = udp_server(argv[1], SERV_PORT_STR, NULL);
    } else {
        err_quit("usage: udpserv01 [ <hostname> ];");
    }
    if (sockfd < 0) {
        err_sys("udp_server error");
    }
    struct sockaddr_storage ss;
    dg_echo(sockfd, (SA *)&ss, sizeof(ss));
}

