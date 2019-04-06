#include "unp.h"
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

#define NBUFF 8

#define ERR_SYS(ret) if ((ret) != 0) { \
            err_sys(#ret " error"); \
        }

#define ERR_THREAD(ret) do { \
            int err; \
            if ((err = (ret)) != 0) { \
                err_exit(err, #ret " error"); \
            } \
        } while (0)

struct shared_data {
    struct {
        char data[BUFFSIZE];
        ssize_t n;
    } buff[NBUFF];
    sem_t mutex, nempty, nstored;
} shared;

int fd;
void *produce(void *arg), *consume(void *arg);

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: mycat2 <pathname>");
    }
    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        err_sys("open error");
    }
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
    int i = 0;
    for (;;) {
        ERR_SYS(sem_wait(&shared.nempty));
        ERR_SYS(sem_wait(&shared.mutex));
        // critical region
        ERR_SYS(sem_post(&shared.mutex));

        shared.buff[i].n = read(fd, shared.buff[i].data, BUFFSIZE);
        if (shared.buff[i].n == 0) {
            ERR_SYS(sem_post(&shared.nstored));
            return NULL;
        }
        if (++i >= NBUFF) {
            i = 0;
        }
        ERR_SYS(sem_post(&shared.nstored));
    }
}

void *consume(void *arg) {
    int i = 0;
    for (;;) {
        ERR_SYS(sem_wait(&shared.nstored));
        ERR_SYS(sem_wait(&shared.mutex));
        // critical region
        ERR_SYS(sem_post(&shared.mutex));

        if (shared.buff[i].n == 0) {
            return NULL;
        }
        ERR_SYS(write(STDOUT_FILENO, shared.buff[i].data, shared.buff[i].n));
        if (++i >= NBUFF) {
            i = 0;
        }
        ERR_SYS(sem_post(&shared.nempty));
    }
}
