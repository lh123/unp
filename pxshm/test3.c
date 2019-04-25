#include "unp.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: test3 <name>");
    }
    shm_unlink(argv[1]);
    int fd1 = shm_open(argv[1], O_RDWR | O_CREAT | O_EXCL, FILEMODE);
    if (fd1 < 0) {
        err_sys("shm_open error");
    }
    if (ftruncate(fd1, sizeof(int)) < 0) {
        err_sys("ftruncate error");
    }
    int fd2 = open("/dev/zero", O_RDONLY);
    if (fd2 < 0) {
        err_sys("open error");
    }
    struct stat statbuf;
    if (fstat(fd2, &statbuf) < 0) {
        err_sys("fstat error");
    }
    pid_t childpid;
    int *ptr1, *ptr2;
    if ((childpid = fork()) < 0) {
        err_sys("fork error");
    } else if (childpid == 0) {
        ptr2 = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
        ptr1 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
        printf("child: shm ptr = %p, zero ptr = %p\n", ptr1, ptr2);
        sleep(5);
        printf("shared memory integer = %d\n", *ptr1);
        exit(0);
    }
    ptr2 = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
    ptr1 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
    printf("parent: shm ptr = %p, zero ptr = %p\n", ptr1, ptr2);
    *ptr1 = 777;
    printf("shared memory integer = %d\n", *ptr1);
}
