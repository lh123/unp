#include "unp.h"
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char **argv) {
    int flags = O_RDWR | O_CREAT;
    int c;
    while ((c = getopt(argc, argv, "e")) != -1) {
        switch (c) {
        case 'e':
            flags |= O_EXCL;
            break;
        default:
            err_quit("unknown option -%c", optopt);
        }
    }
    if (optind != argc - 2) {
        err_quit("usage: shmcreate [ -e ] <name> <length>");
    }
    off_t length = atoi(argv[optind + 1]);
    int fd;
    if ((fd = shm_open(argv[optind], flags, FILEMODE)) < 0) {
        err_sys("shm_open error");
    }
    if (ftruncate(fd, length) < 0) {
        err_sys("ftruncate error");
    }
    char *ptr;
    if ((ptr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        err_sys("mmap error");
    }
}
