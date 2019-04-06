#include "unp.h"
#include <semaphore.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: sempost <name>");
    }
    sem_t *sem;
    if ((sem = sem_open(argv[1], 0)) == SEM_FAILED) {
        err_sys("sem_open error");
    }
    if (sem_post(sem) < 0) {
        err_sys("sem_post error");
    }
    int val;
    if (sem_getvalue(sem, &val) < 0) {
        err_sys("sem_getvalue error");
    }
    printf("value = %d\n", val);
}
