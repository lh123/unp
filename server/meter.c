#include "unp.h"
#include <sys/mman.h>

long *meter(int nchildren) {
    long *ptr = mmap(0, nchildren * sizeof(long), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    if (ptr == NULL) {
        err_sys("mmap error");
    }
    return ptr;
}
