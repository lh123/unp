#include "unp.h"
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

#define NBUFF       10

#define ERR_SYS(ret) if ((ret) != 0) { \
            err_sys(#ret " error"); \
        }

#define ERR_THREAD(ret) do { \
            int err; \
            if ((err = (ret)) != 0) { \
                err_exit(err, #ret " error"); \
            } \
        } while (0)

int nitems;
struct {
    int buff[NBUFF];
    sem_t mutex, nempty, nstored;
} shared;

void *produce(void *arg), *consume(void *arg);

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: prodcons2 <#items>");
    }
    nitems = atoi(argv[1]);
    ERR_SYS(sem_init(&shared.mutex, 0, 1));
    ERR_SYS(sem_init(&shared.nempty, 0, NBUFF));
    ERR_SYS(sem_init(&shared.nstored, 0, 0));

    ERR_THREAD(pthread_setconcurrency(2));
    pthread_t tid_produce, tid_consume;
    ERR_THREAD(pthread_create(&tid_produce, NULL, &produce, NULL));
    ERR_THREAD(pthread_create(&tid_consume, NULL, &consume, NULL));

    ERR_THREAD(pthread_join(tid_produce, NULL));
    ERR_THREAD(pthread_join(tid_consume, NULL));

    ERR_SYS(sem_destroy(&shared.mutex));
    ERR_SYS(sem_destroy(&shared.nempty));
    ERR_SYS(sem_destroy(&shared.nstored));
}

void *produce(void *arg) {
    int i;
    for (i = 0; i < nitems; i++) {
        ERR_SYS(sem_wait(&shared.nempty));
        ERR_SYS(sem_wait(&shared.mutex));
        shared.buff[i % NBUFF] = i;
        ERR_SYS(sem_post(&shared.mutex));
        ERR_SYS(sem_post(&shared.nstored));
    }
    return NULL;
}

void *consume(void *arg) {
    int i;
    for (i = 0; i < nitems; i++) {
        ERR_SYS(sem_wait(&shared.nstored));
        ERR_SYS(sem_wait(&shared.mutex));
        if (shared.buff[i % NBUFF] != i) {
            printf("buff[%d] = %d\n", i, shared.buff[i % NBUFF]);
        }
        ERR_SYS(sem_post(&shared.mutex));
        ERR_SYS(sem_post(&shared.nempty));
    }
    return NULL;
}
