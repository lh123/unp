#include "unp.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#define SEM_NAME "mysem"

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: incr2 <pathname> <#loops>");
    }
    int nloop = atoi(argv[2]);
    int fd = open(argv[1], O_RDWR | O_CREAT, FILEMODE);
    int zero = 0;
    if (write(fd, &zero, sizeof(int)) == -1) {
        err_sys("write error");
    }
    int *ptr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        err_sys("mmap errpr");
    }
    close(fd);

    sem_t *mutex;
    if ((mutex = sem_open(SEM_NAME, O_CREAT | O_EXCL, FILEMODE, 1)) == SEM_FAILED) {
        err_sys("sem_open error");
    }
    if (sem_unlink(SEM_NAME) == -1) {
        err_sys("sem_unlink error");
    }

    setvbuf(stdout, NULL, _IONBF, 0);
    pid_t pid;
    int i;
    if ((pid = fork()) == -1) {
        err_sys("fork error");
    } else if (pid == 0) {
        for (i = 0; i < nloop; i++) {
            if (sem_wait(mutex) == -1) {
                err_sys("sem_wait error");
            }
            printf("child: %d\n", (*ptr)++);
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
        printf("parent: %d\n", (*ptr)++);
        if (sem_post(mutex) == -1) {
            err_sys("sem_post error");
        }
    }
}
