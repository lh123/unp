#include "unp.h"
#include <sys/sem.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    int c;
    int oflag = SVSEM_MODE | IPC_CREAT;
    while ((c = getopt(argc, argv, "e")) != -1) {
        switch (c) {
        case 'e':
            oflag |= IPC_EXCL;
            break;
        case '?':
        default:
            err_quit("unknown option -%c", optopt);
        }
    }
    if (optind != argc - 2) {
        err_quit("usage: semcreate [ -e ] <pathname> <nsems>");
    }
    int nsems = atoi(argv[optind + 1]);
    int semid;
    key_t key;
    if ((key = ftok(argv[optind], 0)) == -1) {
        err_sys("ftok error");
    }
    if ((semid = semget(key, nsems, oflag)) == -1) {
        err_sys("semget error");
    }
}
