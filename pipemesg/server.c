#include "mesg.h"

void server(int readfd, int writefd) {
    struct mymesg mesg;
    mesg.mesg_type = 1;
    ssize_t n = mesg_recv(readfd, &mesg);
    if (n < 0) {
        err_sys("mesg_recv error");
    } else if (n == 0) {
        err_quit("pathname missing");
    }
    mesg.mesg_data[n] = '\0';
    FILE *fp = fopen(mesg.mesg_data, "r");
    if (fp == NULL) {
        snprintf(mesg.mesg_data + n, sizeof(mesg.mesg_data) - n, ": can't open, %s\n", strerror(errno));
        mesg.mesg_len = strlen(mesg.mesg_data);
        if (mesg_send(writefd, &mesg) < 0) {
            err_sys("mesg_send error");
        }
    } else {
        while (fgets(mesg.mesg_data, MAXMESGDATA, fp) != NULL) {
            mesg.mesg_len = strlen(mesg.mesg_data);
            if (mesg_send(writefd, &mesg) < 0) {
                err_sys("mesg_send error");
            }
        }
        if (ferror(fp)) {
            err_sys("fgets error");
        }
        fclose(fp);
    }
    mesg.mesg_len = 0;
    if (mesg_send(writefd, &mesg) < 0) {
        err_sys("mesg_send error");
    }
}
