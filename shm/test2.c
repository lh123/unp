#include "unp.h"
#include <fcntl.h>
#include <sys/mman.h>

#define FILE    "test.data"
#define SIZE    32768

int main(int argc, char **argv) {
    int fd, i;
    char *ptr;

    if ((fd = open(FILE, O_RDWR | O_CREAT | O_TRUNC, FILEMODE)) < 0) {
        err_sys("open error");
    }
    if ((ptr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        err_sys("mmap error");
    }

    for (i = 4096; i <= SIZE; i += 4096) {
        printf("setting file size to %d\n", i);
        if (ftruncate(fd, i) < 0) {
            err_sys("ftruncate error");
        }
        printf("ptr[%d] = %d\n", i - 1, ptr[i - 1]);
    }
}
