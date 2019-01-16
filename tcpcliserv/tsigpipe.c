#include "unp.h"

void sig_pipe(int signo) {
    printf("SIGPIPE received\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: tcpcli <IPaddress>");
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        err_quit("socket error");
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(13);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        err_sys("inet_pton error for %s", argv[1]);
    }

    if (signal(SIGPIPE, sig_pipe) == SIG_ERR) {
        err_sys("signal SIGPIPE error");
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("connect error");
    }

    sleep(2);
    if (write(sockfd, "hello", 5) <= 0) {
        err_sys("write error");
    }
    sleep(2);
    if (write(sockfd, "world", 5) <= 0) {
        err_sys("write error");
    }
}
