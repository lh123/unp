#include "unp.h"
#include <semaphore.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: semwait error");
    }
    sem_t *sem;
    if ((sem = sem_open(argv[1], 0)) == SEM_FAILED) {
        err_sys("sem_open error");
    }
    if (sem_wait(sem) < 0) {
        err_sys("sem_wait error");
    }
    int val;
    if (sem_getvalue(sem, &val) < 0) {
        err_sys("sem_getvalue error");
    }
    printf("pid %ld has semaphore, value = %d\n", (long)getpid(), val);

    pause(); // blocks until killed
}
