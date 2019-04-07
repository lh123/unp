#include "unp.h"
#include <sys/sem.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    int c, flag;
    flag = 0;
    while ((c = getopt(argc, argv, "nu")) != -1) {
        switch (c) {
        case 'n':
            flag |= IPC_NOWAIT;
            break;
        case 'u':
            flag |= SEM_UNDO;
            break;
        case '?':
        default:
            err_quit("unknown option -%c", optopt);
        }
    }
    if (argc - optind < 2) {
        err_quit("usage: semops [ -n ] [ -u ] <pathname> operation ...");
    }
    int semid;
    key_t key;
    if ((key = ftok(argv[optind], 0)) == -1) {
        err_sys("ftok error");
    }
    if ((semid = semget(key, 0, 0)) == -1) {
        err_sys("semget error");
    }
    optind++;
    int nops = argc - optind;

    struct sembuf *ptr = calloc(nops, sizeof(struct sembuf));
    int i;
    for (i = 0; i < nops; i++) {
        ptr[i].sem_num = i;
        ptr[i].sem_op = atoi(argv[optind + i]);
        ptr[i].sem_flg = flag;
    }
    if (semop(semid, ptr, nops) == -1) {
        err_sys("semop error");
    }
}
