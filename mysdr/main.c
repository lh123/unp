#include "mysdr.h"
#include <stddef.h>

#define SAP_NAME "sap.mcast.net"
#define SAP_PORT "9875"

void loop(int sockfd, socklen_t salen);

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);
    if (argc == 1) {
        sockfd = udp_client(SAP_NAME, SAP_PORT, (SA *)&sa, &salen);
    } else if (argc == 4) {
        sockfd = udp_client(argv[1], argv[2], (SA *)&sa, &salen);
    } else {
        err_quit("usage: mysdr [ <mcast-addr> <port#> <interface-name> ]");
    }
    if (sockfd < 0) {
        err_sys("upd_client error");
    }
    const int on = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }
    if (bind(sockfd, (SA *)&sa, salen) < 0) {
        err_sys("bind error");
    }
    if (mcast_join(sockfd, (SA *)&sa, salen, (argc == 4) ? argv[3] : NULL, 0) < 0) {
        err_sys("mcast_join error");
    }
    loop(sockfd, salen);
}

void loop(int sockfd, socklen_t salen) {
    struct sap_packet {
        uint32_t sap_header;
        uint32_t sap_src;
        char sap_data[BUFFSIZE];
    } buf;
    struct sockaddr_storage sa;
    for (;;) {
        socklen_t len = salen;
        ssize_t n = recvfrom(sockfd, &buf, sizeof(buf) - 1, 0, (SA *)&sa, &len);
        if (n < 0) {
            err_sys("recvfrom error");
        }
        if (n > offsetof(struct sap_packet, sap_data)) {
            buf.sap_data[n - offsetof(struct sap_packet, sap_data)] = 0;
        } else {
            buf.sap_data[0] = 0;
        }
        buf.sap_header = ntohl(buf.sap_header);
        printf("From %s hash 0x%04x\n", sock_ntop((SA *)&sa, len), buf.sap_header & SAP_HASH_MASK);
        if (((buf.sap_header & SAP_VERSION_MASK) >> SAP_VERSION_SHIFT) > 1) {
            err_msg("... version field not 1 (0x%08x)", buf.sap_header);
            continue;
        }
        if (buf.sap_header & SAP_IPV6) {
            err_msg("... IPv6");
            continue;
        }
        if (buf.sap_header & (SAP_DELETE | SAP_ENCRYPTED | SAP_COMPRESSED)) {
            err_msg("... can't parse this packet type (0x%08x)", buf.sap_header);
            continue;
        }
        char *p = buf.sap_data + ((buf.sap_header & SAP_AUTHLEN_MASK) >> SAP_AUTHLEN_SHIFT);
        if (strcmp(p, "application/sdp") == 0) {
            p += 16;
        }
        printf("%s\n", p);
    }
}
