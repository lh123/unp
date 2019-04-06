#include "unp.h"
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

#define NBUFF       10
#define SEM_MUTEX   "mutex"
#define SEM_NEMPTY  "nempty"
#define SEM_NSTORED "nstored"

int nitems;
struct {
    int buff[NBUFF];
    sem_t *mutex, *nempty, *nstored;
} shared;

void *produce(void *arg), *consume(void *arg);

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: prodcons1 <#items>");
    }
    nitems = atoi(argv[1]);
    if ((shared.mutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, FILEMODE, 1)) == SEM_FAILED) {
        err_sys("sem_open error");
    }
    if ((shared.nempty = sem_open(SEM_NEMPTY, O_CREAT | O_EXCL, FILEMODE, NBUFF)) == SEM_FAILED) {
        err_sys("sem_open error");
    }
    if ((shared.nstored = sem_open(SEM_NSTORED, O_CREAT | O_EXCL, FILEMODE, 0)) == SEM_FAILED) {
        err_sys("sem_open error");
    }
    int err;
    if ((err = pthread_setconcurrency(2)) != 0) {
        err_exit(err, "pthread_setconcurrency error");
    }
    pthread_t tid_produce, tid_consume;
    if ((err = pthread_create(&tid_produce, NULL, &produce, NULL)) != 0) {
        err_exit(err, "pthread_create error");
    }
    if ((err = pthread_create(&tid_consume, NULL, &consume, NULL)) != 0) {
        err_exit(err, "pthread_create error");
    }
    if ((err = pthread_join(tid_produce, NULL)) != 0) {
        err_exit(err, "pthread_join error");
    }
    if ((err = pthread_join(tid_consume, NULL)) != 0) {
        err_exit(err, "pthread_join error");
    }
    if (sem_unlink(SEM_MUTEX) != 0) {
        err_sys("sem_unlink error");
    }
    if (sem_unlink(SEM_NEMPTY) != 0) {
        err_sys("sem_unlink error");
    }
    if (sem_unlink(SEM_NSTORED) != 0) {
        err_sys("sem_unlink error");
    }
}

void *produce(void *arg) {
    int i;
    for (i = 0; i < nitems; i++) {
        if (sem_wait(shared.nempty) != 0) {
            err_sys("sem_wait error");
        }
        if (sem_wait(shared.mutex) != 0) {
            err_sys("sem_wait error");
        }
        shared.buff[i % NBUFF] = i;
        if (sem_post(shared.mutex) != 0) {
            err_sys("sem_post error");
        }
        if (sem_post(shared.nstored) != 0) {
            err_sys("sem_post error");
        }
    }
    return NULL;
}

void *consume(void *arg) {
    int i;
    for (i = 0; i < nitems; i++) {
        if (sem_wait(shared.nstored) != 0) {
            err_sys("sem_wait error");
        }
        if (sem_wait(shared.mutex) != 0) {
            err_sys("sem_wait error");
        }
        if (shared.buff[i % NBUFF] != i) {
            printf("buff[%d] = %d\n", i, shared.buff[i % NBUFF]);
        }
        if (sem_post(shared.mutex) != 0) {
            err_sys("sem_post error");
        }
        if (sem_post(shared.nempty) != 0) {
            err_sys("sem_post error");
        }
    }
    return NULL;
}
