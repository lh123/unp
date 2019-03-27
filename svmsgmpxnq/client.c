#include "mesg.h"

void client(int readfd, int writefd) {
    struct mymesg mesg;
    snprintf(mesg.mesg_data, MAXMESGDATA, "%d ",readfd);
    size_t len = strlen(mesg.mesg_data);

    char *ptr = mesg.mesg_data + len;

    if (fgets(ptr, MAXMESGDATA - len, stdin) == NULL && ferror(stdin)) {
        err_sys("fgets error");
    }
    len = strlen(mesg.mesg_data);
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
        if (write(STDOUT_FILENO, mesg.mesg_data, n) < 0) {
            err_sys("write error");
        }
    }
    if (n < 0) {
        err_sys("mesg_recv error");
    }
}
