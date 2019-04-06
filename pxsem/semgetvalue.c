#include "unp.h"
#include <semaphore.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: semgetvalue <name>");
    }
    sem_t *sem;
    if ((sem = sem_open(argv[1], 0)) == SEM_FAILED) {
        err_sys("sem_open error");
    }
    int val;
    if (sem_getvalue(sem, &val) < 0) {
        err_sys("sem_getvalue error");
    }
    printf("value = %d\n", val);
}
