#include "cliserv2.h"
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: server2 <name>");
    }
    shm_unlink(argv[1]);
    int fd = shm_open(argv[1], O_RDWR | O_CREAT | O_EXCL, FILEMODE);
    if (fd < 0) {
        err_sys("shm_open error");
    }
    struct shmstruct *ptr;
    if ((ptr = mmap(NULL, sizeof(struct shmstruct), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        err_sys("mmap error");
    }
    if (ftruncate(fd, sizeof(struct shmstruct)) < 0) {
        err_sys("ftruncate error");
    }
    close(fd);

    int index, lastnoverflow, temp;
    long offset;
    for (index = 0; index < NMESG; index++) {
        ptr->msgoff[index] = index * MESGSIZE;
    }
    if (sem_init(&ptr->mutex, 1, 1) < 0) {
        err_sys("sem_init error");
    }
    if (sem_init(&ptr->nempty, 1, NMESG) < 0) {
        err_sys("sem_init error");
    }
    if (sem_init(&ptr->nstored, 1, 0) < 0) {
        err_sys("sem_init error");
    }
    if (sem_init(&ptr->noverflowmutex, 1, 1) < 0) {
        err_sys("sem_init error");
    }

    index = 0;
    lastnoverflow = 0;
    for (;;) {
        if (sem_wait(&ptr->nstored) < 0) {
            err_sys("sem_wait error");
        }
        if (sem_wait(&ptr->mutex) < 0) {
            err_sys("sem_wait error");
        }
        offset = ptr->msgoff[index];
        printf("index = %d: %s\n", index, &ptr->msgdata[offset]);
        if (++index >= NMESG) {
            index = 0;
        }
        if (sem_post(&ptr->mutex) < 0) {
            err_sys("sem_post error");
        }
        if (sem_post(&ptr->nempty) < 0) {
            err_sys("sem_post error");
        }

        if (sem_wait(&ptr->noverflowmutex) < 0) {
            err_sys("sem_wait error");
        }
        temp = ptr->noverflow;
        if (sem_post(&ptr->noverflowmutex) < 0) {
            err_sys("sem_post error");
        }
        if (temp != lastnoverflow) {
            printf("noverflow = %d\n", temp);
            lastnoverflow = temp;
        }
    }
}
