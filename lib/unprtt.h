#ifndef _UNP_RTT_H
#define _UNP_RTT_H

#include "unp.h"

struct rtt_info {
    float rtt_rtt;
    float rtt_srtt;
    float rtt_rttvar;
    float rtt_rto;
    int rtt_nrexmt;
    uint32_t rtt_base;
};

#define RTT_RXTMIN      2
#define RTT_RXTMAX      60
#define RTT_MAXNREXMT   3

// void rtt_debug(struct rtt_info *);
void rtt_init(struct rtt_info *);
void rtt_newpack(struct rtt_info *);
int rtt_start(struct rtt_info *);
void rtt_stop(struct rtt_info *, uint32_t);
int rtt_timeout(struct rtt_info *);
uint32_t rtt_ts(struct rtt_info *);

extern int rtt_d_flag;

#endif // _UNP_RTT_H