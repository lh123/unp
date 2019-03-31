#include "unp.h"
#include <fcntl.h>

#define SEQFILE "seqno.out"

#define ERR_SYS(func_call) if ((func_call) < 0) { \
            err_sys(#func_call " error"); \
        }

// void my_lock(int), my_unlock(int);
#define my_lock(fd) (writew_lock(fd, 0, SEEK_SET, 0))
#define my_unlock(fd) (un_lock(fd, 0, SEEK_SET, 0))

int main(int argc, char **argv) {
    pid_t pid = getpid();
    int fd = open(SEQFILE, O_RDWR, FILEMODE);
    if (fd < 0) {
        err_sys("open error");
    }

    int i;
    char line[MAXLINE + 1];
    long seqno;
    for (i = 0; i < 20; i++) {
        my_lock(fd);
        ERR_SYS(lseek(fd, 0, SEEK_SET));
        ssize_t n = read(fd, line, MAXLINE);
        if (n < 0) {
            err_sys("read error");
        }
        line[n] = '\0';
        n = sscanf(line, "%ld\n", &seqno);
        printf("%s: pid = %ld, seq# = %ld\n", argv[0], (long)pid, seqno);

        seqno++;

        snprintf(line, sizeof(line), "%ld\n", seqno);
        ERR_SYS(lseek(fd, 0L, SEEK_SET));
        ERR_SYS(write(fd, line, strlen(line)));

        my_unlock(fd);
    }
}
