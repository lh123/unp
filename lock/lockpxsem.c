#include "unp.h"
#include <fcntl.h>
#include <semaphore.h>

#define LOCKFILE    "pxsemlock"

sem_t *locksem;
int initflag;

void my_lock(int fd) {
    if (initflag == 0) {
        if (locksem = sem_open(LOCKFILE, O_CREAT, FILEMODE, 1) == SEM_FAILED) {
            err_sys("sem_open error");
        }
        initflag = 1;
    }
    if (sem_wait(locksem) != 0) {
        err_sys("sem_wait error");
    }
}

void my_unlock(int fd) {
    if (sem_post(locksem) != 0) {
        err_sys("sem_post error");
    }
}
