#ifndef _UNP_IFI_H
#define _UNP_IFI_H

#include "unp.h"
#include <net/if.h>

#define IFI_NAME 16 // same as IFNAMSIZ in <net/if.h>
#define IFI_HADDR 8 // allow for 64-bit EUI-64 in future

struct ifi_info {
    char ifi_name[IFI_NAME];    // interface name, null-terminated
    short ifi_mtu;              // interface MTU
    short ifi_flags;            // IFF_xxx constants from <net/if.h>
    short ifi_myflags;          // our own IFF_xxx flags
    struct sockaddr *ifi_addr;  // primary address
    struct sockaddr *ifi_brdaddr;   // broadcast address
    struct sockaddr *ifi_dstaddr;   // destination address
    struct ifi_info *ifi_next;      // next of these structures
};

#define IFI_ALIAS 1 // ifi_addr is an alias

struct ifi_info *get_ifi_info(int family, int doaliases);
void free_ifi_info(struct ifi_info *ifihead);

#endif // _UNP_IFI_H
