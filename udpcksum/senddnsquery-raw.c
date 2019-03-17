#include "udpcksum.h"

void send_dns_query(void) {
    char buf[sizeof(struct udpiphdr) + 100];
    char *ptr = buf + sizeof(struct udpiphdr);
    *(uint16_t *)ptr = htons(1234);
    ptr += 2;
    *(uint16_t *)ptr = htons(0x0100);
    ptr += 2;
    *(uint16_t *)ptr = htons(1);
    ptr += 2;
    *(uint16_t *)ptr = 0;
    ptr += 2;
    *(uint16_t *)ptr = 0;
    ptr += 2;
    *(uint16_t *)ptr = 0;
    ptr += 2;

    memcpy(ptr, "\001a\005baidu\003com\000", 13);
    ptr += 13;
    *(uint16_t *)ptr = htons(1);
    ptr += 2;
    *(uint16_t *)ptr = htons(1);
    ptr += 2;

    size_t nbytes = (ptr - buf) - sizeof(struct udpiphdr);
    udp_write(buf, nbytes);
    if (verbose) {
        printf("sent: %ld bytes of data\n", nbytes);
    }
}
