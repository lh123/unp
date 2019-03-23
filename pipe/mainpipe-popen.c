#include "unp.h"

int main(int argc, char **argv) {
    char buff[MAXLINE], command[MAXLINE + 4];
    if (fgets(buff, MAXLINE, stdin) == NULL) {
        err_sys("fgets error");
    }
    if (ferror(stdin)) {
        err_sys("fgets error");
    }
    size_t n = strlen(buff);
    if (buff[n - 1] == '\n') {
        n--;
    }
    snprintf(command, sizeof(command), "cat %s", buff);
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        err_sys("popen error");
    }
    while (fgets(buff, MAXLINE, fp) != NULL) {
        if (fputs(buff, stdout) == EOF) {
            err_sys("fputs error");
        }
    }
    if (ferror(fp)) {
        err_sys("fgets error");
    }
    pclose(fp);
}
