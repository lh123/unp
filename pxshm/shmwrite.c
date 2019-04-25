#include "unp.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: shmwrite <name>");
    }
    int fd;
    if ((fd = shm_open(argv[1], O_RDWR, 0)) < 0) {
        err_sys("shm_open error");
    }
    struct stat statbuf;
    if (fstat(fd, &statbuf) < 0) {
        err_sys("fstat error");
    }
    unsigned char *ptr;
    if ((ptr = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        err_sys("mmap error");
    }
    close(fd);

    int i;
    for (i = 0; i < statbuf.st_size; i++) {
        *ptr++ = i % 256;
    }
}
