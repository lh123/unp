#include "unp.h"
#include <sys/stat.h>
#include <sys/sem.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: semrmid <pathname>");
    }
    key_t key;
    if ((key = ftok(argv[1], 0)) == -1) {
        err_sys("ftok error");
    }
    int semid;
    if ((semid = semget(key, 0, 0)) == -1) {
        err_sys("semget error");
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        err_sys("semctl error");
    }
}
