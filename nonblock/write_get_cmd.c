#include "web.h"

void write_get_cmd(struct file *fptr) {
    char line[MAXLINE];
    int n = snprintf(line, sizeof(line), GET_CMD, fptr->f_name);
    if (writen(fptr->f_fd, line, n) < 0) {
        err_sys("writen error");
    }
    printf("wrote %d bytes for %s\n", n, fptr->f_name);
    fptr->f_flags = F_READING;
    
    FD_SET(fptr->f_fd, &rset);
    if (fptr->f_fd > maxfd) {
        maxfd = fptr->f_fd;
    }
}
