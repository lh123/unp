#include "unp.h"
#include <pthread.h>

#define NLOOP 5000

int counter;

void *doit(void *arg);

int main(int argc, char **argv) {
    pthread_t tidA, tidB;
    int err;
    if ((err = pthread_create(&tidA, NULL, doit, NULL)) != 0) {
        err_exit(err, "pthread_create error");
    }
    if ((err = pthread_create(&tidB, NULL, doit, NULL)) != 0) {
        err_exit(err, "pthread_create error");
    }

    if ((err = pthread_join(tidA, NULL)) != 0) {
        err_exit(err, "pthread_join error");
    }
    if ((err = pthread_join(tidB, NULL)) != 0) {
        err_exit(err, "pthread_join error");
    }
}

void *doit(void *arg) {
    int i;
    for (i = 0; i < NLOOP; i++) {
        int val = counter;
        printf("%d: %d\n", (int)pthread_self(), val + 1);
        counter = val + 1;
    }
    return NULL;
}
