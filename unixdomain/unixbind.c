#include "unp.h"
#include <sys/un.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: unixbind <pathname>");
    }
    int sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sockfd < 0) {
        err_sys("socket error");
    }
    unlink(argv[1]);

    struct sockaddr_un addr1, addr2;
    memset(&addr1, 0, sizeof(addr1));
    addr1.sun_family = AF_LOCAL;
    strncpy(addr1.sun_path, argv[1], sizeof(addr1.sun_path) - 1);

    if (bind(sockfd, (SA *)&addr1, sizeof(addr1)) < 0) {
        err_sys("bind error");
    }

    socklen_t len = sizeof(addr2);
    if (getsockname(sockfd, (SA *)&addr2, &len) < 0) {
        err_sys("getsockname error");
    }
    printf("bound name = %s, returned len = %d\n", addr2.sun_path, len);
}
