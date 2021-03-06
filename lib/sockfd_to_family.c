#include "unp.h"

int sockfd_to_family(int sockfd) {
    struct sockaddr_storage ss;
    socklen_t len = sizeof(ss);
    if (getsockname(sockfd, (struct sockaddr *)&ss, &len) < 0) {
        return -1;
    }
    return ss.ss_family;
}
