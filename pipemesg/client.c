#include "mesg.h"

void client(int readfd, int writefd) {
    struct mymesg mesg;
    if (fgets(mesg.mesg_data, MAXMESGDATA, stdin) == NULL) {
        err_sys("fgets error");
    }
    size_t len = strlen(mesg.mesg_data);
    if (mesg.mesg_data[len - 1] == '\n') {
        len--;
    }
    mesg.mesg_len = len;
    mesg.mesg_type = 1;
    if (mesg_send(writefd, &mesg) < 0) {
        err_sys("mesg_send error");
    }
    ssize_t n;
    while ((n = mesg_recv(readfd, &mesg)) > 0) {
        if (write(STDOUT_FILENO, mesg.mesg_data, mesg.mesg_len) < 0) {
            err_sys("write error");
        }
    }
    if (n < 0) {
        err_sys("mesg_recv error");
    }
}
