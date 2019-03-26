#include "unp.h"
#include "mqueue.h"

int mq_getattr(mqd_t mqd, struct mq_attr *mqstat) {
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
    mqstat->mq_flags = mqinfo->mqi_flags;
    mqstat->mq_maxmsg = attr->mq_maxmsg;
    mqstat->mq_msgsize = attr->mq_msgsize;
    mqstat->mq_curmsgs = attr->mq_curmsgs;

    pthread_mutex_unlock(&mqhdr->mqh_lock);
    return 0;
}
