#include "unp.h"
#include <fcntl.h>
#include <semaphore.h>

int main(int argc, char **argv) {
    int flags = O_RDWR | O_CREAT;
    unsigned int value = 1;
    int c;
    while ((c = getopt(argc, argv, "ei:")) != -1) {
        switch (c) {
        case 'e':
            flags |= O_EXCL;
            break;
        case 'i':
            value = atoi(optarg);
            break;
        case '?':
        default:
            err_quit("unknown option -%c", optopt);
        }
    }
    if (optind != argc - 1) {
        err_quit("usage: semcreate [ -e ] [ -i initalvalue ] <name>");
    }
    sem_t *sem;
    if ((sem = sem_open(argv[optind], flags, FILEMODE, value)) == SEM_FAILED) {
        err_sys("sem_open error");
    }
    if (sem_close(sem) < 0) {
        err_sys("sem_close error");
    }
}
