#include "unp.h"
#include <netdb.h>

u_char *inet_srcrt_init(int type);
int inet_srcrt_add(char *hostptr);
void inet_srcrt_print(u_char *ptr, int len);
void str_cli(FILE *fp, int sockfd);

int main(int argc, char **argv) {
    if (argc < 2) {
        err_quit("usage: tcpcli01 [ -[gG] <hostname> ... ] <hostname>");
    }
    opterr = 0; // don't want getopt() writing to stderr
    int c;
    u_char *ptr = NULL;
    while ((c = getopt(argc, argv, "gG")) != -1) {
        switch (c) {
        case 'g':
            if (ptr) {
                err_quit("can't use both -g and -G");
            }
            ptr = inet_srcrt_init(0);
            break;
        case 'G':
            if (ptr) {
                err_quit("can't use both -g and -G");
            }
            ptr = inet_srcrt_init(1);
            break;
        case '?':
        default:
            err_quit("unrecognized option: -%c", optopt);
        }
    }
    int len = 0;
    if (ptr) {
        while (optind < argc - 1) {
            len = inet_srcrt_add(argv[optind++]);
        }
    } else if (optind < argc - 1) {
        err_quit("need -g or -G to specify route");
    }
    if (optind != argc - 1) {
        err_quit("missing <hostname>");
    }
    struct addrinfo *ai = host_serv(argv[optind], SERV_PORT_STR, AF_INET, SOCK_STREAM);
    int sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sockfd < 0) {
        err_sys("socket error");
    }
    if (ptr) {
        len = inet_srcrt_add(argv[optind]); // dst at end
        if (setsockopt(sockfd, IPPROTO_IP, IP_OPTIONS, ptr, len) < 0) {
            err_sys("setsockopt error");
        }
        free(ptr);
    }
    if (connect(sockfd, ai->ai_addr, ai->ai_addrlen) < 0) {
        err_sys("connect error");
    }
    str_cli(stdin, sockfd);
}

void str_cli(FILE *fp, int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE + 1];
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (writen(sockfd, sendline, strlen(sendline)) < 0) {
            err_sys("writen error");
        }
        ssize_t n;
        if ((n = read(sockfd, recvline, MAXLINE)) < 0) {
            err_sys("read error");
        }
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
    if (ferror(fp)) {
        err_sys("fgets error");
    }
}
