#include "unp.h"

void str_cli(FILE *fp_arg, int sockfd_arg);

int main(int argc, char **argv) {
    int sockfd;
    if (argc == 2) {
        sockfd = tcp_connect(NULL, argv[1]);
    } else if (argc == 3) {
        sockfd = tcp_connect(argv[1], argv[2]);
    } else {
        err_quit("usage: tcpcli01 [hostname/IPaddress] <service/port>");
    }
    str_cli(stdin, sockfd);
}