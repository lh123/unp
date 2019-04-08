#include "unp.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

struct shared {
    sem_t mutex;
    int count;
} shared;

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: incr3 <#loops>");
    }
    int nloop = atoi(argv[1]);
    struct shared *ptr;
    if ((ptr = mmap(NULL, sizeof(shared), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0)) == MAP_FAILED) {
        err_sys("mmap error");
    }
    if (sem_init(&ptr->mutex, 1, 1) < 0) {
        err_sys("sem_init error");
    }
    setbuf(stdout, NULL);

    pid_t pid;
    int i;
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        for (i = 0; i < nloop; i++) {
            if (sem_wait(&ptr->mutex) < 0) {
                err_sys("sem_wait error");
            }
            printf("child: %d\n", ptr->count++);
            if (sem_post(&ptr->mutex) < 0) {
                err_sys("sem_post error");
            }
        }
        exit(0);
    }
    for (i = 0; i < nloop; i++) {
        if (sem_wait(&ptr->mutex) < 0) {
            err_sys("sem_wait error");
        }
        printf("parent: %d\n", ptr->count++);
        if (sem_post(&ptr->mutex) < 0) {
            err_sys("sem_post error");
        }
    }
}