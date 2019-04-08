#include "unp.h"
#include <fcntl.h>
#include <sys/mman.h>

#define DECLARE_MAX(type) type max_##type(type a, type b) { return a > b ? a : b; }

DECLARE_MAX(int);
DECLARE_MAX(long);

int main(int argc, char **argv) {
    if (argc != 4) {
        err_quit("usage: test1 <pathname> <filesize> <mmapsize>");
    }
    size_t filesize, mmapsize, pagesize;
    filesize = atoi(argv[2]);
    mmapsize = atoi(argv[3]);

    int fd, i;
    if ((fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, FILEMODE)) < 0) {
        err_sys("open error");
    }
    if (lseek(fd, filesize - 1, SEEK_SET) < 0) {
        err_sys("lseek error");
    }
    if (write(fd, "", 1) < 0) {
        err_sys("write error");
    }
    char *ptr;
    if ((ptr = mmap(NULL, mmapsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        err_sys("mmap error");
    }
    close(fd);

    errno = 0;
    if ((pagesize = sysconf(_SC_PAGESIZE)) < 0 && errno != 0) {
        err_sys("sysconf error");
    }
    printf("PAGESIZE = %ld\n", (long)pagesize);

    for (i = 0; i < max_long(filesize, mmapsize); i+= pagesize) {
        printf("ptr[%d] = %d\n", i, ptr[i]);
        ptr[i] = 1;
        printf("ptr[%d] = %d\n", (int)(i + pagesize - 1), ptr[i + pagesize - 1]);
        ptr[i + pagesize - 1] = 1;
    }
    printf("ptr[%d] = %d\n", i, ptr[i]);
}
