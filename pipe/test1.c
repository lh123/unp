#include "unp.h"
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: test1 <pathname>");
    }
    if (mkfifo(argv[1], FILEMODE) < 0) {
        err_sys("mkfifo error");
    }
    int fd[2];
    fd[0] = open(argv[1], O_RDONLY | O_NONBLOCK);
    fd[1] = open(argv[1], O_WRONLY | O_NONBLOCK);
    if (fd[0] < 0 || fd[1] < 0) {
        err_sys("open error");
    }
    struct stat info;
    if (fstat(fd[0], &info) < 0) {
        err_sys("fstat error");
    }
    printf("fd[0]: st_size = %ld\n", (long)info.st_size);
    if (fstat(fd[1], &info) < 0) {
        err_sys("fstat error");
    }
    printf("fd[1]: st_size = %ld\n", (long)info.st_size);
    char buff[7];
    if (write(fd[1], buff, sizeof(buff)) < 0) {
        err_sys("write error");
    }

    if (fstat(fd[0], &info) < 0) {
        err_sys("fstat error");
    }
    printf("fd[0]: st_size = %ld\n", (long)info.st_size);
    if (fstat(fd[1], &info) < 0) {
        err_sys("fstat error");
    }
    printf("fd[1]: st_size = %ld\n", (long)info.st_size);
}
