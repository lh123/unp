#include "unp.h"
#include <fcntl.h>

int my_open(const char *pathname, int mode);

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: mycat <pathname>");
    }

    int fd = my_open(argv[1], O_RDONLY);
    if (fd < 0) {
        err_sys("cannot open %s", argv[1]);
    }

    ssize_t n;
    char buff[BUFSIZ];
    while ((n = read(fd, buff, BUFSIZ)) > 0) {
        if (write(STDOUT_FILENO, buff, n) < 0) {
            err_sys("write error");
        }
    }
    if (n < 0) {
        err_sys("read error");
    }
}
