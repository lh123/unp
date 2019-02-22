#include "unp.h"
#include <netdb.h>

int main(int argc, char **argv) {
    char str[INET_ADDRSTRLEN];

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
            pptr = hptr->h_addr_list;
            for (;*pptr != NULL; pptr++) {
                printf("\taddress: %s\n", inet_ntop(AF_INET, *pptr, str, sizeof(str)));
            }
            break;
        default:
            err_ret("unknown address type");
        }
    }
}
