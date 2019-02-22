#include "unp.h"
#include <netdb.h>

struct command_opt {
    char name;
    char desc[256];
    int hasvalue;
    char value[256];
};

int parse_commandline(int argc, char **argv, struct command_opt *opts) {
    int index;
    struct command_opt *popt;
    for (index = 1; index < argc;) {
        if (argv[index][0] != '-') {
            printf("unknown opt '-%c'\n", argv[index][1]);
            return -1;
        }
        for (popt = opts; popt->name != 0; popt++) {
            if (popt->name == argv[index][1]) {
                break;
            }
        }
        if (popt->name == 0) {
            printf("unknown opt '-%c'\n", argv[index][1]);
            return -1;
        }
        char *ptr = NULL;
        if (argv[index][2] != 0) {
            ptr = &argv[index][2];
        } else if (argv[index][2] == 0) {
            if (index + 1 < argc && argv[index + 1][0] != '-') {
                index++;
                ptr = &argv[index][0];
            }
        }
        if (ptr == NULL && popt->hasvalue) {
            printf("arg '-%c' hasn't value\n", popt->name);
            return -1;
        } else if (ptr != NULL && popt->hasvalue == 0) {
            printf("arg '-%c' should not has value\n", popt->name);
            return -1;
        }
        if (ptr != NULL) {
            strncpy(popt->value, ptr, sizeof(popt->value) - 1);
            popt->value[sizeof(popt->value) - 1] = 0;
        } else {
            popt->value[0] = '1';
        }
        index++;
    }
    return 0;
}

static struct command_opt opts[] = {
    {'f', "family", 1, ""},
    {'c', "canonnane name", 0, "0"},
    {'h', "hostname", 1, ""},
    {'s', "server name", 1, ""},
    {'t', "socket type", 1, ""},
    {'p', "passive", 0, "0"},
    {0, "", 0, ""}
};

int main(int argc, char **argv) {
    if (parse_commandline(argc, argv, opts) < 0) {
        return 1;
    }
    char *family, *host, *server, *socktype;
    int canonname, passive;

    struct command_opt *popt;
    for (popt = opts; popt->name != 0; popt++) {
        switch (popt->name) {
        case 'f':
            family = popt->value;
            break;
        case 'c':
            canonname = popt->value[0] == '0' ? 0 : 1;
            break;
        case 'h':
            if (popt->value[0] == 0) {
                host = NULL;
            } else {
                host = popt->value;
            }
            break;
        case 's':
            if (popt->value[0] == 0) {
                server = NULL;
            } else {
                server = popt->value;
            }
            break;
        case 't':
            socktype = popt->value;
            break;
        case 'p':
            passive = popt->value[0] == '0' ? 0 : 1;
            break;
        default:
            err_ret("unknown opt");
        }
    }

    struct addrinfo hint, *addrlist;
    memset(&hint, 0, sizeof(hint));
    if (family[0] == 0) {
        hint.ai_family = AF_UNSPEC;
    } else if (strcmp(family, "inet") == 0) {
        hint.ai_family = AF_INET;
    } else if (strcmp(family, "inet6") == 0) {
        hint.ai_family = AF_INET6;
    } else {
        err_quit("invaild arg value '-%c': %s\n", 'f', family);
    }

    if (canonname) {
        hint.ai_flags |= AI_CANONNAME;
    }
    if (passive) {
        hint.ai_flags |= AI_PASSIVE;
    }

    if (strcmp(socktype, "stream") == 0) {
        hint.ai_socktype = SOCK_STREAM;
    } else if (strcmp(socktype, "dgram") == 0) {
        hint.ai_socktype = SOCK_DGRAM;
    } else if (strcmp(socktype, "seqpacket") == 0) {
        hint.ai_socktype = SOCK_SEQPACKET;
    } else if (socktype[0] != 0) {
        err_quit("invaild arg value '-%c': %s\n", 't', socktype);
    }

    if (getaddrinfo(host, server, &hint, &addrlist) < 0) {
        err_sys("getaddrinfo error");
    }

    struct addrinfo *addr;
    for (addr = addrlist; addr != NULL; addr = addr->ai_next) {
        printf("socket(");
        switch (addr->ai_family) {
        case AF_INET:
            printf("AF_INET");
            break;
        case AF_INET6:
            printf("AF_INET6");
            break;
        case AF_UNIX:
            printf("AF_UNIX");
            break;
        case AF_UNSPEC:
            printf("AF_UNSPEC");
            break;
        default:
            printf("unknown");
        }
        printf(", ");
        switch (addr->ai_socktype) {
        case SOCK_STREAM:
            printf("SOCK_STREAM");
            break;
        case SOCK_DGRAM:
            printf("SOCK_DGRAM");
            break;
        case SOCK_SEQPACKET:
            printf("SOCK_SEQPACKET");
            break;
        case SOCK_RAW:
            printf("SOCK_RAW");
            break;
        default:
            printf("unknown");
        }
        printf(", %d)", addr->ai_protocol);
        if (canonname && addr->ai_canonname != NULL) {
            printf(", ai_canonname = %s", addr->ai_canonname);
        }
        printf("\n");

        if (addr->ai_family == AF_INET) {
            char str[INET_ADDRSTRLEN];
            struct sockaddr_in *ipv4addr =(struct sockaddr_in *)addr->ai_addr;
            printf("\taddress: %s:%d\n", inet_ntop(AF_INET, &ipv4addr->sin_addr, str, sizeof(str)), ntohs(ipv4addr->sin_port));
        } else if (addr->ai_family == AF_INET6) {
            char str[INET6_ADDRSTRLEN];
            struct sockaddr_in6 *ipv6addr =(struct sockaddr_in6 *)addr->ai_addr;
            printf("\taddress: %s:%d\n", inet_ntop(AF_INET6, &ipv6addr->sin6_addr, str, sizeof(str)), ntohs(ipv6addr->sin6_port));
        }
    }

    freeaddrinfo(addrlist);
}
