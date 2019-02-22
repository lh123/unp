#include "unp.h"
#include <netdb.h>

int main(int argc, char **argv) {
    char str[INET6_ADDRSTRLEN];
    while (--argc > 0) {
        char *ptr = *(++argv);
        struct hostent *hptr = gethostbyname(ptr);
        if (hptr == NULL) {
            err_msg("gethostbyname error for host: %s: %s", ptr, hstrerror(h_errno));
            continue;
        }

        printf("official hostname: %s\n", hptr->h_name);
        char **pptr;
        for (pptr = hptr->h_aliases; *pptr != NULL; pptr++) {
            printf("\talias: %s\n", *pptr);
        }
        switch (hptr->h_addrtype) {
        case AF_INET:
        case AF_INET6:
            pptr = hptr->h_addr_list;
            for (; *pptr != NULL; pptr++) {
                printf("\taddress: %s\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
                struct hostent *hptr1;
                if ((hptr1 = gethostbyaddr(*pptr, hptr->h_length, hptr->h_addrtype)) == NULL) {
                    printf("\t(gethostbyaddr failed)\n");
                } else if (hptr1->h_name != NULL) {
                    printf("\tname = %s\n", hptr1->h_name);
                } else {
                    printf("\t(no hostname returned by gethostbyaddr)\n");
                }
            }
            break;
        default:
            err_ret("unknown address type");
        }
    }
}
