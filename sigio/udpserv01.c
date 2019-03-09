#include "unp.h"

void dg_echo(int sockfd_arg, SA *pcliaddr, socklen_t clilen_arg);

int main(int argc, char **argv) {
    int sockfd;
    socklen_t len;
    if (argc == 2) {
        sockfd = udp_server(NULL, argv[1], &len);
    } else if (argc == 3) {
        sockfd = udp_server(argv[1], argv[2], &len);
    } else {
        err_quit("usage: udpserv01 [hostname/IPaddress] <port>");
    }
    struct sockaddr_storage cliaddr;
    dg_echo(sockfd, (SA *)&cliaddr, len);
}
