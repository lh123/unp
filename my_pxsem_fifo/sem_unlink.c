#include "unp.h"
#include "semaphore.h"

int my_sem_unlink(const char *pathname) {
    return unlink(pathname);
}
