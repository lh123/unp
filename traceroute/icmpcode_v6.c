#include "trace.h"

const char *icmpcode_v6(int code) {
    static char errbuf[100];
    switch (code) {
    case ICMP6_DST_UNREACH_NOROUTE:
        return "no route to host";
    case ICMP6_DST_UNREACH_ADMIN:
        return "administratively prohibited";
    case ICMP6_DST_UNREACH_ADDR:
        return "address unreachable";
    case ICMP6_DST_UNREACH_NOPORT:
        return "port unreachable";
    default:
        sprintf(errbuf, "[unkown code %d]", code);
        return errbuf;
    }
}
