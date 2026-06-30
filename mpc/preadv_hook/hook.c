/* lustre-mpc — LD_PRELOAD intercept library */
#define _GNU_SOURCE
#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <dlfcn.h>
#include "common/types.h"
#include "cache_daemon/ring_buffer.h"
#include "cache_daemon/markov_model.h"
#include "common/ipc.h"

#define MAX_TRACKED   64

struct tracked_fd {
    int               fd;
    lmpc_fid_t        fid;
    char              src_path[256];
    lmpc_region_t     last_region;
    uint32_t          gen;
};

static struct tracked_fd tr_fds[MAX_TRACKED] = {{0}};
static int n_tracked = 0;
static int g_socket    = -1;      /* Unix socket connected to daemon  */

off_t (*real_pread)(int, void*, size_t, off_t)   = NULL;
ssize_t (*real_preadv)(int, const struct iovec*, int, off_t) = NULL;

void __attribute__((constructor)) hook_init(void)
{
    real_pread = (off_t(*)(int,void*,size_t,off_t))dlsym(RTLD_NEXT, "pread");
    real_preadv = (ssize_t(*)(int,const struct iovec*,int,off_t))dlsym(RTLD_NEXT, "preadv");
}

static void track_fd(int fd, lmpc_fid_t fid, const char *sp)
{
    if (n_tracked >= MAX_TRACKED) return;
    tr_fds[n_tracked].fd = fd;
    tr_fds[n_tracked].fid = fid;
    snprintf(tr_fds[n_tracked].src_path, sizeof(tr_fds[n_tracked].src_path), "%s", sp);
    tr_fds[n_tracked].last_region = 0;
    tr_fds[n_tracked].gen = 0;
    n_tracked++;
}

off_t my_pread(int fd, void *buf, size_t count, off_t offset)
{
    /* Find tracking entry for this FD */
    int ti = -1;
    for (int i = 0; i < n_tracked; i++)
        if (tr_fds[i].fd == fd) { ti = i; break; }

    if (ti < 0) {
        /* Not a tracked FD → passthrough directly */
        return real_pread(fd, buf, count, offset);
    }

    lmpc_region_t region = offset_to_region(offset);
    off_t inner_offset   = offset % (off_t)MPC_CHUNK_SIZE;

    /* Try lookup in local ring buffer */
    int slot = -1;
    /* Eventually: rb_lookup(ringbuf, tr_fds[ti].fid, region, &slot);
       For now fall through to Lustre and notify daemon: */
    ssize_t rc = real_pread(fd, buf, count, offset);

    /* Record Markov transition from previous to current region */
    lmpc_region_t prev = tr_fds[ti].last_region;
    tr_fds[ti].last_region = region;
    (void)prev;

    return (rc > 0) ? rc : 0;
}

ssize_t my_preadv(int fd, const struct iovec *vecs, int nvecs, off_t offset)
{
    ssize_t total = 0;
    for (int v = 0; v < nvecs; v++) {
        ssize_t rn = my_pread(fd, vecs[v].iov_base,
                              (size_t)vecs[v].iov_len, offset);
        if (rn <= 0) break;
        total += rn;
        offset += vecs[v].iov_len;
    }
    return total;
}
