#include "unp.h"
#include <sys/sem.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        err_quit("usage: semsetvalues <pathname> [ values ... ]");
    }
    key_t key;
    if ((key = ftok(argv[1], 0)) == -1) {
        err_sys("ftok error");
    }
    int semid;
    if ((semid = semget(key, 0, 0)) == -1) {
        err_sys("semget error");
    }
    union semun arg;
    struct semid_ds seminfo;
    arg.buf = &seminfo;
    if (semctl(semid, 0, IPC_STAT, arg) == -1) {
        err_sys("semctl error");
    }
    int nsems = arg.buf->sem_nsems;

    if (argc != nsems + 2) {
        err_quit("%d semaphores in set, %d value specified", nsems, argc - 2);
    }
    unsigned short *ptr = calloc(nsems, sizeof(unsigned short));
    arg.array = ptr;
    int i;
    for (i = 0; i < nsems; i++) {
        ptr[i] = atoi(argv[i + 2]);
    }
    if (semctl(semid, nsems, SETALL, arg) == -1) {
        err_sys("semctl error");
    }
}
