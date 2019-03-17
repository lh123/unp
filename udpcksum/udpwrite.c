#include "udpcksum.h"

uint16_t in_cksum(uint16_t *addr, int len);

int rawfd;

void open_output(void) {
    const int on = 1;

    rawfd = socket(dest->sa_family, SOCK_RAW, IPPROTO_UDP);
    if (rawfd < 0) {
        err_sys("socket SOCK_RAW error");
    }
    if (setsockopt(rawfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }
}

void udp_write(char *buf, int userlen) {
    struct udpiphdr *ui = (struct udpiphdr *)buf;
    struct ip *ip = (struct ip *)buf;

    memset(ui, 0, sizeof(*ui));
    // add 8 to userlen for pesudoheader length
    ui->ip.ip_len = htons((uint16_t)(sizeof(struct udphdr) + userlen));
    // then add 28 for IP datagram length
    userlen += sizeof(struct udpiphdr);

    ui->ip.ip_p = IPPROTO_UDP;
    ui->ip.ip_src = ((struct sockaddr_in *)local)->sin_addr;
    ui->ip.ip_dst = ((struct sockaddr_in *)dest)->sin_addr;
    ui->udp.uh_sport = ((struct sockaddr_in *)local)->sin_port;
    ui->udp.uh_dport = ((struct sockaddr_in *)dest)->sin_port;
    ui->udp.uh_ulen = ui->ip.ip_len;
    
    if (zerosum == 0) {
        if ((ui->udp.uh_sum = in_cksum((uint16_t *)ui, userlen)) == 0) {
            ui->udp.uh_sum = 0xFFFF;
        }
    }
    ip->ip_v = IPVERSION;
    ip->ip_hl = sizeof(struct ip) >> 2; // /4
    ip->ip_tos = 0;
    ip->ip_len = htons(userlen);
    ip->ip_id = 0;
    ip->ip_off = 0;
    ip->ip_ttl = TTL_OUT;

    if (sendto(rawfd, buf, userlen, 0, dest, destlen) < 0) {
        err_sys("sendto error");
    }
}
