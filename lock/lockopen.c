#include "unp.h"
#include <fcntl.h>

#define LOCKFILE    "/tmp/seqno.lock"

void my_lock(int fd) {
    int tempfd;
    while ((tempfd = open(LOCKFILE, O_RDWR | O_CREAT | O_EXCL, FILEMODE)) < 0) {
        if (errno != EEXIST) {
            err_sys("open error for lock file");
        }
        // someone else has the lock, loop around and try again
    }
    close(tempfd); // opened the file, we have the lock
}

void my_unlock(int fd) {
    if (unlink(LOCKFILE) < 0) { // release lock by removing file
        err_sys("unlink error for lock file");
    }
}
