#include "unp.h"
#include "mqueue.h"

#include <sys/mman.h>

int mq_close(mqd_t mqd) {
    struct mq_info *mqinfo = mqd;
    if (mqinfo->mqi_magic != MQI_MAGIC) {
        errno = EBADF;
        return -1;
    }
    struct mq_hdr *mqhdr = mqinfo->mqi_hdr;
    struct mq_attr *attr = &mqhdr->mqh_attr;

    if (mq_notify(mqd, NULL) != 0) {
        return -1;
    }
    long msgsize = MSGSIZE(attr->mq_msgsize);
    long filesize = sizeof(struct mq_hdr) + (attr->mq_maxmsg * (sizeof(struct msg_hdr) + msgsize));

    if (munmap(mqinfo->mqi_hdr, filesize) == -1) {
        return -1;
    }
    mqinfo->mqi_magic = 0;
    free(mqinfo);
    return 0;
}
