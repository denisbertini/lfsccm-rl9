/* lustre-mpc — Ring buffer implementation */
#include "cache_daemon/ring_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

static int count_valid(const struct ring_buf *r)
{
    int v = 0;
    for (int i = 0; i < r->capacity; i++)
        if (r->slots[i].valid) v++;
    return v;
}

struct ring_buf *rb_new(int scnt, const char *cdir)
{
    if (!scnt || !cdir) return NULL;
    
    struct ring_buf *rb = calloc(1, sizeof(*rb));
    if (!rb) return NULL;
    
    rb->slots = calloc(scnt, sizeof(struct slot_entry));
    if (!rb->slots) { free(rb); return NULL; }
    
    rb->capacity = scnt;
    rb->chunk_sz = MPC_CHUNK_SIZE;
    rb->head = 0;
    rb->tail = 0;
    pthread_rwlock_init(&rb->lock, NULL);
    
    for (int i = 0; i < scnt; i++) {
        snprintf(rb->slots[i].path, sizeof(rb->slots[i].path),
                 "%s/slot_%d", cdir, i);
        int fd = open(rb->slots[i].path, O_CREAT|O_WRONLY|O_TRUNC, 0600);
        if (fd < 0) { fprintf(stderr, "[rb] mknod fail %s\n", rb->slots[i].path); goto fail; }
        if (ftruncate(fd, (off_t)rb->chunk_sz)) { close(fd); goto fail; }
        close(fd);
    }
    return rb;
  
fail:
    for (int i = 0; i < scnt && rb->slots[i].path[0]; i++) unlink(rb->slots[i].path);
    free(rb->slots); free(rb);
    return NULL;
}

void rb_free(struct ring_buf *rb)
{
    if (!rb) return;
    for (int i = 0; i < rb->capacity; i++) unlink(rb->slots[i].path);
    pthread_rwlock_destroy(&rb->lock);
    free(rb->slots);
    free(rb);
}

int rb_lookup(struct ring_buf *rb, lmpc_fid_t fid, lmpc_region_t reg, int *slot_idx)
{
    pthread_rwlock_wrlock(&rb->lock);
    for (int i = 0; i < rb->capacity; i++) {
        if (rb->slots[i].valid &&
            rb->slots[i].ext.fid == fid &&
            rb->slots[i].ext.region == reg) {
            *slot_idx = i;
            pthread_rwlock_unlock(&rb->lock);
            return MPC_CACHE_HIT;
        }
    }
    pthread_rwlock_unlock(&rb->lock);
    return MPC_CACHE_MISS;
}

int rb_reserve(struct ring_buf *rb, int *out_idx)
{
    pthread_rwlock_wrlock(&rb->lock);
    
    /* evict head if ring is full */
    while (count_valid(rb) >= rb->capacity) {
        struct slot_entry *victim = &rb->slots[rb->head];
        memset(victim, 0, sizeof(*victim));
        rb->head = (rb->head + 1) % rb->capacity;
    }
    
    *out_idx = rb->tail;
    memset(&rb->slots[*out_idx], 0, sizeof(rb->slots[*out_idx]));
    rb->tail = (rb->tail + 1) % rb->capacity;
    
    pthread_rwlock_unlock(&rb->lock);
    return 0;
}

int rb_commit(struct ring_buf *rb, int si, struct lmpc_extent e)
{
    pthread_rwlock_wrlock(&rb->lock);
    rb->slots[si].ext = e;
    rb->slots[si].valid = 1;
    pthread_rwlock_unlock(&rb->lock);
    return 0;
}

int rb_pread(struct ring_buf *rb, int si, off_t inner_off, void *buf, size_t n)
{
    int fd = open(rb->slots[si].path, O_RDONLY);
    if (fd < 0) return MPC_ERR_IO;
    ssize_t rc = pread(fd, buf, n, inner_off);
    close(fd);
    return (rc == (ssize_t)n) ? MPC_OK : MPC_ERR_IO;
}

int rb_valid_n(const struct ring_buf *r)
{
    int v = 0;
    for (int i = 0; i < r->capacity; i++)
        if (r->slots[i].valid) v++;
    return v;
}
