#include "unp.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

struct shmstruct {
    int count;
};

sem_t *mutex;

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: server1 <shmname> <semname>");
    }
    shm_unlink(argv[1]);
    int fd;
    if ((fd = shm_open(argv[1], O_RDWR | O_CREAT | O_EXCL, FILEMODE)) < 0) {
        err_sys("shm_open error");
    }
    if (ftruncate(fd, sizeof(struct shmstruct)) < 0) {
        err_sys("ftruncate error");
    }
    struct shmstruct *ptr;
    if ((ptr = mmap(NULL, sizeof(struct shmstruct), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        err_sys("mmap error");
    }
    close(fd);
    sem_unlink(argv[2]);
    if ((mutex = sem_open(argv[2], O_CREAT | O_EXCL, FILEMODE, 1)) == SEM_FAILED) {
        err_sys("sem_open error");
    }
    if (sem_close(mutex) < 0) {
        err_sys("sem_close error");
    }
}
