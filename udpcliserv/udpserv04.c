#include "unp.h"
#include <signal.h>

typedef struct sockaddr SA;

void dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen);
static void recvfrom_int(int signal);
static volatile int count;

int main(int argc, char **argv) {
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        err_sys("socket error");
    }

    if (bind(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }

    struct sockaddr_in cliaddr;
    dg_echo(sockfd, (SA *)&cliaddr, sizeof(cliaddr));
}

void dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen) {
    char mesg[MAXLINE];

    struct sigaction act;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = recvfrom_int;

    if (sigaction(SIGINT, &act, NULL) < 0) {
        err_sys("sigaction for SIGINT error");
    }

    for (;;) {
        socklen_t len = clilen;
        if (recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len) < 0) {
            err_sys("recvfrom error");
        }
        count++;
    }
}

static void recvfrom_int(int signal) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}
