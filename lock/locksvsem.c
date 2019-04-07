#include "unp.h"
#include <sys/sem.h>
#include <sys/stat.h>

#define LOCK_PATH   "/tmp/svsemlock"
#define MAX_TRIES   10

int semid, initflag;
struct sembuf postop, waitop;

void my_lock(int fd) {
    if (initflag == 0) {
        int oflag = IPC_CREAT | IPC_EXCL | SVSEM_MODE;
        key_t key;
        if ((key = ftok(LOCK_PATH, 0)) == -1) {
            err_sys("ftok error");
        }
        union semun arg;
        if ((semid = semget(key, 1, oflag)) >= 0) {
            arg.val = 1;
            if (semctl(semid, 0, SETVAL, arg) == -1) {
                err_sys("semctl error");
            }
        } else if (errno == EEXIST) {
            if ((semid = semget(key, 0, 0)) == -1) {
                err_sys("semget error");
            }
            struct semid_ds seminfo;
            arg.buf = &seminfo;
            int i;
            for (i = 0; i < MAX_TRIES; i++) {
                if (semctl(semid, 0, IPC_STAT, arg) == -1) {
                    err_sys("semctl error");
                }
                if (arg.buf->sem_otime != 0) {
                    goto init;
                }
                sleep(1);
            }
            err_quit("semget OK, but semaphore not initialized");
        } else {
            err_sys("semget error");
        }
init:
        initflag = 1;
        postop.sem_num = 0;
        postop.sem_op = 1;
        postop.sem_flg = SEM_UNDO;
        waitop.sem_num = 0;
        waitop.sem_op = -1;
        waitop.sem_flg = SEM_UNDO;
    }
    if (semop(semid, &waitop, 1) == -1) {
        err_sys("semop error");
    }
}

void my_unlock(int fd) {
    if (semop(semid, &postop, 1) == -1) {
        err_sys("semop error");
    }
}
