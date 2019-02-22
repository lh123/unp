#include "unp.h"
#include <netdb.h>

int main(int argc, char **argv) {
    char str[INET_ADDRSTRLEN];
    int ret;
    while (argc-- > 1) {
        char *ptr = *(++argv);
        in_addr_t addr;
        if ((ret = inet_pton(AF_INET, ptr, &addr)) < 0) {
            err_ret("inet_pton error for %s", ptr);
            continue;
        } else if (ret == 0) {
            err_msg("invaild address: %s", ptr);
            continue;
        }
        struct hostent *hptr = gethostbyaddr(&addr, sizeof(addr), AF_INET);
        if (hptr == NULL) {
            err_msg("gethostbyaddr error for address: %s: %s", ptr, hstrerror(h_errno));
            continue;
        }

        printf("hostname: %s\n", hptr->h_name);
        char **pptr;
        for (pptr = hptr->h_aliases; *pptr != NULL; pptr++) {
            printf("\talias: %s\n", *pptr);
        }
        switch (hptr->h_addrtype) {
        case AF_INET:
            for (pptr = hptr->h_addr_list; *pptr != NULL; pptr++) {
                printf("\taddress: %s\n", inet_ntop(AF_INET, *pptr, str, sizeof(str)));
            }
            break;
        default:
            err_ret("unknown address type");
        }
    }
}
