#include "unp.h"
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>

struct shmstruct {
    int count;
};

sem_t *mutex;

int main(int argc, char **argv) {
    if (argc != 4) {
        err_quit("usage: client1 <shmname> <semname> <#loops>");
    }
    int fd, nloop;

    nloop = atoi(argv[3]);
    if ((fd = shm_open(argv[1], O_RDWR, FILEMODE)) < 0) {
        err_sys("shm_open error");
    }
    struct shmstruct *ptr;
    if ((ptr = mmap(NULL, sizeof(struct shmstruct), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        err_sys("mmap error");
    }
    close(fd);

    if ((mutex = sem_open(argv[2], 0)) == SEM_FAILED) {
        err_sys("sem_open error");
    }

    pid_t pid = getpid();
    int i;
    for (i = 0; i < nloop; i++) {
        if (sem_wait(mutex) < 0) {
            err_sys("sem_wait error");
        }
        printf("pid %ld: %d\n", (long)pid, ptr->count++);
        if (sem_post(mutex) < 0) {
            err_sys("sem_post error");
        }
    }
}
