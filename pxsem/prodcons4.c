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

int nitems, nproducers, nconsumers;
struct {
    int buff[NBUFF];
    int nput;
    int nputval;
    int nget;
    int ngetval;
    sem_t mutex, nempty, nstored;
} shared;

int min(int a, int b) {
    return a < b ? a : b;
}

void *produce(void *arg), *consume(void *arg);

int main(int argc, char **argv) {
    if (argc != 4) {
        err_quit("usage: prodcons2 <#items> <#producers> <#consumers>");
    }
    nitems = atoi(argv[1]);
    nproducers = min(atoi(argv[2]), MAXNTHREADS);
    nconsumers = min(atoi(argv[3]), MAXNTHREADS);

    ERR_SYS(sem_init(&shared.mutex, 0, 1));
    ERR_SYS(sem_init(&shared.nempty, 0, NBUFF));
    ERR_SYS(sem_init(&shared.nstored, 0, 0));

    ERR_THREAD(pthread_setconcurrency(nproducers + nconsumers));
    pthread_t tid_produce[MAXNTHREADS], tid_consume[MAXNTHREADS];
    int i, prodcount[MAXNTHREADS], conscount[MAXNTHREADS];
    for (i = 0; i < nproducers; i++) {
        prodcount[i] = 0;
        ERR_THREAD(pthread_create(&tid_produce[i], NULL, &produce, &prodcount[i]));
    }
    for (i = 0; i < nconsumers; i++) {
        conscount[i] = 0;
        ERR_THREAD(pthread_create(&tid_consume[i], NULL, &consume, &conscount[i]));
    }

    for (i = 0; i < nproducers; i++) {
        ERR_THREAD(pthread_join(tid_produce[i], NULL));
        printf("producer count[%d] = %d\n", i, prodcount[i]);
    }
    for (i = 0; i < nconsumers; i++) {
        ERR_THREAD(pthread_join(tid_consume[i], NULL));
        printf("consumer count[%d] = %d\n", i, conscount[i]);
    }

    ERR_SYS(sem_destroy(&shared.mutex));
    ERR_SYS(sem_destroy(&shared.nempty));
    ERR_SYS(sem_destroy(&shared.nstored));
}

void *produce(void *arg) {
    for (;;) {
        ERR_SYS(sem_wait(&shared.nempty));
        ERR_SYS(sem_wait(&shared.mutex));

        if (shared.nput >= nitems) {
            ERR_SYS(sem_post(&shared.nstored)); // let consumers terminate
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
    for (;;) {
        ERR_SYS(sem_wait(&shared.nstored));
        ERR_SYS(sem_wait(&shared.mutex));

        if (shared.nget >= nitems) {
            ERR_SYS(sem_post(&shared.mutex)); // wake other consumer
            ERR_SYS(sem_post(&shared.nstored));
            return NULL;
        }
        i = shared.nget % NBUFF;
        if (shared.buff[i] != shared.ngetval) {
            printf("error: buff[%d] = %d\n", i, shared.buff[i]);
        }
        shared.nget++;
        shared.ngetval++;

        ERR_SYS(sem_post(&shared.mutex));
        ERR_SYS(sem_post(&shared.nempty));
        *(int *)arg += 1;
    }
}
