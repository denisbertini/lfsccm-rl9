/* lustre-mpc — Prefetch worker thread pool */
#ifndef LUSTRE_MPC_PREFETCH_H
#define LUSTRE_MPC_PREFETCH_H
#include <pthread.h>
#include "common/types.h"

struct ring_buf;                /* opaque forward-declared */

#define PF_WORKERS_MAX  4
#define FETCH_Q_SZ     64

struct Fetch_task {
    lmpc_fid_t     fid;
    lmpc_region_t  region;
    off_t          offset_in_lustre;
    size_t         length;
    int            slot_idx;
};

struct pf_pool {
    int               nw;
    pthread_t       tids[PF_WORKERS_MAX];
    struct ring_buf *rb;
    const char      *lustre_file;
    struct Fetch_task work_q[FETCH_Q_SZ];
    int volatile q_head;        /* enqueuer           */
    int volatile q_tail;        /* consumers drain    */
    int volatile shutdown_flag;
};

int   pf_init(struct pf_pool *pf, const char *lustre_path, struct ring_buf *rb, int n);
void  pf_enqueue(struct pf_pool *pf, struct Fetch_task task);
void  pf_shutdown_and_join(struct pf_pool *pf);

#endif /* LUSTRE_MPC_PREFETCH_H */
