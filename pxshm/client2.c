#include "cliserv2.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>

void sleep_us(int usec);

int main(int argc, char **argv) {
    int fd, i, nloop, nusec;
    struct shmstruct *ptr;
    pid_t pid;
    char mesg[MESGSIZE];
    long offset;

    if (argc != 4) {
        err_quit("usage: client2 <name> <#loops> <#usec>");
    }
    nloop = atoi(argv[2]);
    nusec = atoi(argv[3]);
    
    if ((fd = shm_open(argv[1], O_RDWR, FILEMODE)) < 0) {
        err_sys("shm_open error");
    }
    if ((ptr = mmap(NULL, sizeof(struct shmstruct), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        err_sys("mmap error");
    }
    close(fd);

    pid = getpid();
    for (i = 0; i < nloop; i++) {
        sleep_us(nusec);
        snprintf(mesg, MESGSIZE, "pid %ld: message %d", (long)pid, i);

        if (sem_trywait(&ptr->nempty) < 0) {
            if (errno == EAGAIN) {
                if (sem_wait(&ptr->noverflowmutex) < 0) {
                    err_sys("sem_wait error");
                }
                ptr->noverflow++;
                if (sem_post(&ptr->noverflowmutex) < 0) {
                    err_sys("sem_post error");
                }
                continue;
            } else {
                err_sys("sem_trywait error");
            }
        }
        if (sem_wait(&ptr->mutex) < 0) {
            err_sys("sem_wait error");
        }
        offset = ptr->msgoff[ptr->nput];
        if (++(ptr->nput) >= NMESG) {
            ptr->nput = 0;
        }
        if (sem_post(&ptr->mutex) < 0) {
            err_sys("sem_post error");
        }
        strcpy(&ptr->msgdata[offset], mesg);
        if (sem_post(&ptr->nstored) < 0) {
            err_sys("sem_post error");
        }
    }
}

void sleep_us(int usec) {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = usec;
    select(0, NULL, NULL, NULL, &tv);
}
