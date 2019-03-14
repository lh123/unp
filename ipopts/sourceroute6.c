#include "unp.h"

void inet6_srcrt_print(void *ptr) {
    int segments = inet6_rth_segments(ptr);
    if (segments < 0) {
        err_sys("inet6_rth_segments error");
    }
    printf("received source route: ");
    int i;
    char str[INET6_ADDRSTRLEN];
    for (i = 0; i < segments; i++) {
        printf("%s ", inet_ntop(AF_INET6, inet6_rth_getaddr(ptr, i), str, sizeof(str)));
    }
    printf("\n");
}

