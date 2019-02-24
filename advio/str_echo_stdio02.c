#include "unp.h"

void str_echo(int sockfd) {
    char line[MAXLINE];

    FILE *fpin, *fpout;
    if ((fpin = fdopen(sockfd, "r")) == NULL) {
        err_sys("fdopen error");
    }
    if ((fpout = fdopen(sockfd, "w")) == NULL) {
        err_sys("fdopen error");
    }

    while (fgets(line, MAXLINE, fpin) != NULL) {
        if (fputs(line, fpout) == EOF) {
            err_sys("fputs error");
        }
    }

    if (ferror(fpin)) {
        err_sys("fgets error");
    }
}
