#include "unp.h"
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>

#define SEM_NAME    "mysem"

int count = 0;

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: incr1 <#loops>");
    }
    int nloop = atoi(argv[1]);
    sem_t *mutex;
    if ((mutex = sem_open(SEM_NAME, O_CREAT | O_EXCL, FILEMODE, 1)) == SEM_FAILED) {
        err_sys("sem_open error");
    }
    if (sem_unlink(SEM_NAME) == -1) {
        err_sys("sem_unlink error");
    }

    setbuf(stdout, NULL); // stdout is unbuffered
    pid_t pid;
    int i;
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        for (i = 0; i < nloop; i++) {
            if (sem_wait(mutex) == -1) {
                err_sys("sem_wait error");
            }
            printf("child: %d\n", count++);
            if (sem_post(mutex) == -1) {
                err_sys("sem_post error");
            }
        }
        exit(0);
    }
    for (i = 0; i < nloop; i++) {
        if (sem_wait(mutex) == -1) {
            err_sys("sem_wait error");
        }
        printf("parent: %d\n", count++);
        if (sem_post(mutex) == -1) {
            err_sys("sem_post error");
        }
    }
}
