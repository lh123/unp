#include "unp.h"
#include <sys/shm.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    int oflag, c, id;
    size_t length;
    key_t key;
    char *ptr;

    oflag = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | IPC_CREAT;
    while ((c = getopt(argc, argv, "e")) != -1) {
        switch (c) {
        case 'e':
            oflag |= IPC_EXCL;
            break;
        case '?':
        default:
            err_quit("unknown option '-%c'", optopt);
        }
    }
    if (optind != argc - 2) {
        err_quit("usage: shmget [ -e ] <pathname> <length>");
    }
    length = atoi(argv[optind + 1]);
    if ((key = ftok(argv[optind], 0)) < 0) {
        err_sys("ftok error");
    }
    if ((id = shmget(key, length, oflag)) < 0) {
        err_sys("shmget error");
    }
    if ((ptr = shmat(id, NULL, 0)) == (char *)-1) {
        err_sys("shmat error");
    }
}
