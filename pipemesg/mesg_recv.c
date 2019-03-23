#include "mesg.h"

ssize_t mesg_recv(int fd, struct mymesg *mptr) {
    ssize_t n = read(fd, mptr, MESGHDRSIZE);
    if (n <= 0) {
        return n;
    } else if (n != MESGHDRSIZE) {
        err_quit("message header: expected %d, got %d", MESGHDRSIZE, (int)n);
    }
    size_t len = mptr->mesg_len;
    if (len > 0) {
        if ((n = read(fd, mptr->mesg_data, len)) != len) {
            err_quit("message data: expected %d, got %d", (int)len, (int)n);
        }
    }
    return len;
}
