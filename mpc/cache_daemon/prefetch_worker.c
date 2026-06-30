/* lustre-mpc — Prefetch worker pool implementation */
#define _GNU_SOURCE
#include "cache_daemon/prefetch_worker.h"
#include "cache_daemon/ring_buffer.h"
#include "common/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

static void *worker_routine(void *arg)
{
    struct pf_pool *pf = (struct pf_pool *)arg;

    while (!pf->shutdown_flag) {
        int head = __atomic_load_n(&pf->q_head, __ATOMIC_RELAXED);
        int tail = __atomic_load_n(&pf->q_tail, __ATOMIC_ACQUIRE);
        
        if (head <= tail) {  /* queue empty */
            usleep(100 * 1000);  /* 100 ms poll sleep */
            continue;
        }
        
        int idx = __atomic_add_fetch(&pf->q_tail, 1, __ATOMIC_ACQ_REL) - 1;
        if (idx >= FETCH_Q_SZ) {
            /* Safety bound, shouldn't normally hit */
            continue;
        }
        struct Fetch_task task = pf->work_q[idx];
        
        /* Execute fetch: Lustre -> local slot */
        int lfd = open(pf->lustre_file, O_RDONLY);
        if (lfd < 0) continue;
        
        char *buf = malloc(task.length > 4096 ? 4096 : task.length);
        if (!buf) { close(lfd); continue; }
        
        ssize_t total_read = 0;
        size_t remaining = task.length;
        off_t cursor = task.offset_in_lustre;
        const char *spath = pf->rb->slots[task.slot_idx].path;
        int ofd = open(spath, O_WRONLY);
        if (ofd < 0) { free(buf); close(lfd); continue; }
        
        while (remaining > 0 && !pf->shutdown_flag) {
            size_t chunk = remaining > 4096 ? 4096 : remaining;
            ssize_t n = pread(lfd, buf, chunk, cursor);
            if (n <= 0) break;
            pwrite(ofd, buf, (size_t)n, cursor - task.offset_in_lustre + total_read);
            total_read += n;
            cursor += (off_t)n;
            remaining -= (size_t)n;
        }
        
        (void)total_read;
        free(buf);
        close(lfd);
        close(ofd);
    }
    return NULL;
}

int pf_init(struct pf_pool *pf, const char *lustre_path, struct ring_buf *rb, int nw)
{
    memset(pf, 0, sizeof(*pf));
    pf->rb = rb;
    pf->lustre_file = strdup(lustre_path ?: "");
    pf->nw = (nw > PF_WORKERS_MAX) ? PF_WORKERS_MAX : (nw < 1 ? 1 : nw);
    pf->q_head = 0;
    pf->q_tail = 0;
    
    for (int i = 0; i < pf->nw; i++) {
        if (pthread_create(&pf->tids[i], NULL, worker_routine, pf))
            fprintf(stderr, "[prefetch] thread %d failed\n", i);
    }
    return 0;
}

void pf_enqueue(struct pf_pool *pf, struct Fetch_task tsk)
{
    int head = __atomic_load_n(&pf->q_head, __ATOMIC_RELAXED);
    int tail = __atomic_load_n(&pf->q_tail, __ATOMIC_ACQUIRE);
    
    /* Overflow: discard oldest if full, never block caller */
    if ((head - tail + FETCH_Q_SZ) % FETCH_Q_SZ == 0) {
        __atomic_store_n(&pf->q_tail, (tail + 1) % FETCH_Q_SZ, __ATOMIC_RELAXED);
    }
    
    pf->work_q[head % FETCH_Q_SZ] = tsk;
    __atomic_add_fetch(&pf->q_head, 1, __ATOMIC_RELEASE);
}

void pf_shutdown_and_join(struct pf_pool *pf)
{
    pf->shutdown_flag = 1;
    for (int i = 0; i < pf->nw; i++)
        pthread_join(pf->tids[i], NULL);
    free((void *)pf->lustre_file);
}
