#include "unp.h"
#include <sys/sem.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: semgetvalues <pathname>");
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
    ushort *ptr = calloc(nsems, sizeof(ushort));
    arg.array = ptr;
    if (semctl(semid, 0, GETALL, arg) == -1) {
        err_sys("semctl error");
    }
    int i;
    for (i = 0; i < nsems; i++) {
        printf("semval[%d] = %d\n", i, ptr[i]);
    }
}
