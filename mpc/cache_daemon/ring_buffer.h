/* lustre-mpc — Ring buffer with LRU eviction over bounded /tmp slots          */
#ifndef LUSTRE_MPC_RINGBUF_H
#define LUSTRE_MPC_RINGBUF_H

#include <pthread.h>
#include "common/types.h"

struct slot_entry {
    struct lmpc_extent ext;       /* owning file + region + gen              */
    int                valid;      /* 1 if holds data for this generation    */
    char               path[256]; /* absolute /tmp/.../slot_N                */
};

struct ring_buf {
    struct slot_entry *slots;
    int                head;        /* oldest -> next evict target                     */
    int                tail;        /* newest (next write position)                    */
    int                capacity;    /* total allocated                                 */
    size_t             chunk_sz;    /* bytes per slot (MPC_CHUNK_SIZE)              */
    pthread_rwlock_t   lock;
};

struct ring_buf *rb_new(int slot_count, const char *cache_dir);
void            rb_free(struct ring_buf *r);
int             rb_lookup(struct ring_buf *r, lmpc_fid_t f, lmpc_region_t reg, int *sidx);
int             rb_reserve(struct ring_buf *r, int *out_sidx);        /* get wr idx, evict if full */
int             rb_commit(struct ring_buf *r, int sidx, struct lmpc_extent e);  /* mark valid     */
int             rb_pread(struct ring_buf *r, int sidx, off_t off_in_slot, void *buf, size_t n);
int             rb_valid_n(const struct ring_buf *r);

#endif /* LUSTRE_MPC_RINGBUF_H */
