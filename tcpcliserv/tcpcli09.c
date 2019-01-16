#include "unp.h"
#include "sum.h"

void str_cli(FILE *fp, int sockfd);

int main(int argc, char **argv) {
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGPIPE, &act, NULL) < 0) {
        err_sys("sigaction for SIGPIPE error");
    }

    int servfd = socket(AF_INET, SOCK_STREAM, 0);
    if (servfd < 0) {
        err_sys("socket error");
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        err_sys("inet_pton error for %s", argv[1]);
    }

    if (connect(servfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("connect error");
    }

    str_cli(stdin, servfd);
}

void str_cli(FILE *fp, int sockfd) {
    char sendline[MAXLINE];
    struct args args;
    struct result result;
    ssize_t n;
    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        if (sscanf(sendline, "%ld%ld", &args.arg1, &args.arg2) != 2) {
            printf("invalid input: %s", sendline);
            continue;
        }
        if (writen(sockfd, &args, sizeof(args)) < 0) {
            err_sys("writen error");
        }
        if ((n = readn(sockfd, &result, sizeof(result))) < 0) {
            err_sys("readn error");
        } else if (n == 0) {
            err_quit("str_cli: server terminated prematurely");
        }
        printf("%ld\n", result.sum);
    }
}
