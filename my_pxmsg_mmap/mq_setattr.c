#include "unp.h"
#include "mqueue.h"

#include <fcntl.h>
#include <sys/stat.h>

int mq_setattr(mqd_t mqd, struct mq_attr *mqstat, struct mq_attr *omqstat) {
    struct mq_info *mqinfo = mqd;
    if (mqinfo->mqi_magic != MQI_MAGIC) {
        errno = EBADF;
        return -1;
    }
    struct mq_hdr *mqhdr = mqinfo->mqi_hdr;
    struct mq_attr *attr = &mqhdr->mqh_attr;

    int n;
    if ((n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) {
        errno = n;
        return -1;
    }
    if (omqstat != NULL) {
        omqstat->mq_flags = mqinfo->mqi_flags;
        omqstat->mq_maxmsg = attr->mq_maxmsg;
        omqstat->mq_msgsize = attr->mq_msgsize;
        omqstat->mq_curmsgs = attr->mq_curmsgs;
    }
    if (mqstat->mq_flags & O_NONBLOCK) {
        mqinfo->mqi_flags |= O_NONBLOCK;
    } else {
        mqinfo->mqi_flags &= ~O_NONBLOCK;
    }
    pthread_mutex_unlock(&mqhdr->mqh_lock);
    return 0;
}
