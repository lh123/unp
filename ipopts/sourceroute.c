#include "unp.h"
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netdb.h>

static u_char *optr;        // pointer into options being formed
static u_char *lenptr;      // pointer to length byte in SSR option
static int ocnt;            // count of # addresses

u_char *inet_srcrt_init(int type) {
    optr = malloc(44);
    memset(optr, 0, 44);
    ocnt = 0;
    *optr++ = IPOPT_NOP;
    *optr++ = type ? IPOPT_SSRR : IPOPT_LSRR;
    lenptr = optr++;
    *optr++ = 4;
    return optr - 4;
}

int inet_srcrt_add(char *hostptr) {
    if (ocnt > 9) {
        err_quit("too many source routes with: %s", hostptr);
    }
    struct addrinfo *ai = host_serv(hostptr, NULL, AF_INET, 0);
    struct sockaddr_in *sin = (struct sockaddr_in *)ai->ai_addr;
    memcpy(optr, &sin->sin_addr, sizeof(struct in_addr));
    freeaddrinfo(ai);

    optr += sizeof(struct in_addr);
    ocnt++;
    int len = 3 + (ocnt * sizeof(struct in_addr));
    *lenptr = len;
    return len + 1; // size for setsockopt (+1 for IPOPT_NOP)
}

void inet_srcrt_print(u_char *ptr, int len) {
    struct in_addr hop1;
    memcpy(&hop1, ptr, sizeof(struct in_addr));
    ptr += sizeof(struct in_addr);
    while (*ptr == IPOPT_NOP) {
        ptr++;
    }
    u_char c = *ptr;
    if (c == IPOPT_LSRR) {
        printf("received LSRR: ");
    } else if (c == IPOPT_SSRR) {
        printf("received SSRR: ");
    } else {
        printf("received option type %d\n", c);
        return;
    }
    char str[INET_ADDRSTRLEN];
    printf("%s ", inet_ntop(AF_INET, &hop1, str, sizeof(str)));
    ptr++;
    len = *ptr - sizeof(struct in_addr);
    ptr++; // skip pointer
    while (len > 0) {
        printf("%s ", inet_ntop(AF_INET, ptr, str, sizeof(str)));
        len -= sizeof(struct in_addr);
        ptr += sizeof(struct in_addr);
    }
    printf("\n");
}
