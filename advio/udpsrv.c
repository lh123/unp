#include "unp.h"

void dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: udpsrv <hostname/IPaddress> <service/port>");
    }
    socklen_t servlen;
    int sockfd = udp_server(argv[1], argv[2], &servlen);
    
    struct sockaddr_storage cliaddr;
    dg_echo(sockfd, (SA *)&cliaddr, servlen);
}

