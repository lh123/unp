#include "unp.h"
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

#define NBUFF       10
#define MAXNTHREADS 100

#define ERR_SYS(ret) if ((ret) != 0) { \
            err_sys(#ret " error"); \
        }

#define ERR_THREAD(ret) do { \
            int err; \
            if ((err = (ret)) != 0) { \
                err_exit(err, #ret " error"); \
            } \
        } while (0)

int nitems, nproducers;
struct {
    int buff[NBUFF];
    int nput;
    int nputval;
    sem_t mutex, nempty, nstored;
} shared;

int min(int a, int b) {
    return a < b ? a : b;
}

void *produce(void *arg), *consume(void *arg);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: prodcons2 <#items> <#producers>");
    }
    nitems = atoi(argv[1]);
    nproducers = min(atoi(argv[2]), MAXNTHREADS);

    ERR_SYS(sem_init(&shared.mutex, 0, 1));
    ERR_SYS(sem_init(&shared.nempty, 0, NBUFF));
    ERR_SYS(sem_init(&shared.nstored, 0, 0));

    ERR_THREAD(pthread_setconcurrency(nproducers + 1));
    pthread_t tid_produce[MAXNTHREADS], tid_consume;
    int i, count[MAXNTHREADS];
    for (i = 0; i < nproducers; i++) {
        count[i] = 0;
        ERR_THREAD(pthread_create(&tid_produce[i], NULL, &produce, &count[i]));
    }
    ERR_THREAD(pthread_create(&tid_consume, NULL, &consume, NULL));

    for (i = 0; i < nproducers; i++) {
        ERR_THREAD(pthread_join(tid_produce[i], NULL));
        printf("count[%d] = %d\n", i, count[i]);
    }
    ERR_THREAD(pthread_join(tid_consume, NULL));

    ERR_SYS(sem_destroy(&shared.mutex));
    ERR_SYS(sem_destroy(&shared.nempty));
    ERR_SYS(sem_destroy(&shared.nstored));
}

void *produce(void *arg) {
    for (;;) {
        ERR_SYS(sem_wait(&shared.nempty));
        ERR_SYS(sem_wait(&shared.mutex));

        if (shared.nput >= nitems) {
            ERR_SYS(sem_post(&shared.mutex));
            ERR_SYS(sem_post(&shared.nempty));
            return NULL;
        }
        shared.buff[shared.nput % NBUFF] = shared.nputval;
        shared.nput++;
        shared.nputval++;

        ERR_SYS(sem_post(&shared.mutex));
        ERR_SYS(sem_post(&shared.nstored));
        *(int *)arg += 1;
    }
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
