#include "unp.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: shmread <name>");
    }
    int fd;
    if ((fd = shm_open(argv[1], O_RDONLY, 0)) < 0) {
        err_sys("shm_open error");
    }
    struct stat statbuf;
    if (fstat(fd, &statbuf) < 0) {
        err_sys("fstat error");
    }
    unsigned char *ptr;
    if ((ptr = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        err_sys("mmap error");
    }
    close(fd);

    int i;
    unsigned char c;
    for (i = 0; i < statbuf.st_size; i++) {
        if ((c = *ptr++) != (i % 256)) {
            err_ret("ptr[%d] = %d", i, c);
        }
    }
}
