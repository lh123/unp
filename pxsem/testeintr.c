#include "unp.h"
#include <semaphore.h>
#include <fcntl.h>

#define NAME "testeintr"

static void sig_alrm(int signo);

int main(int argc, char **argv) {
    sem_t *sem1;
    if ((sem1 = sem_open(NAME, O_RDWR | O_CREAT | O_EXCL, FILEMODE, 0)) == SEM_FAILED) {
        err_sys("sem_open error");
    }
    struct sigaction act;
    act.sa_handler = sig_alrm;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGALRM, &act, NULL) == -1) {
        err_sys("sigaction error");
    }

    alarm(2);
    if (sem_wait(sem1) == 0) {
        printf("sem_wait returned 0?\n");
    } else {
        err_ret("sem_wait error");
    }
    sem_close(sem1);
    sem_unlink(NAME);

    sem_t sem2;
    if (sem_init(&sem2, 0, 0) == -1) {
        err_sys("sem_init error");
    }
    alarm(2);
    if (sem_wait(&sem2) == 0) {
        printf("sem_wait returned 0?\n");
    } else {
        err_ret("sem_wait error");
    }
    sem_destroy(&sem2);
}

static void sig_alrm(int signo) {
    printf("SIGALRM caught\n");
}
