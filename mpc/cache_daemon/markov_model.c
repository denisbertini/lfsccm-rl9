/* lustre-mpc — Sparse first-order Markov prediction engine */
#include "cache_daemon/markov_model.h"
#include <stdlib.h>
#include <string.h>

#define MC_CAP_0      256
#define MC_EDGE_INIT   4

static struct m_row *find_or_create(struct markov_chain *mc, lmpc_region_t src)
{
    for (size_t i = 0; i < mc->row_n; i++)
        if (mc->rows[i].from == src) return &mc->rows[i];

    if (mc->row_n >= mc->row_cap) {
        mc->row_cap *= 2;
        mc->rows = realloc(mc->rows, mc->row_cap * sizeof(*mc->rows));
    }
    struct m_row *r = &mc->rows[mc->row_n++];
    memset(r, 0, sizeof(*r));
    r->from = src;
    r->edges = calloc(MC_EDGE_INIT, sizeof(*r->edges));
    r->edge_cap = MC_EDGE_INIT;
    return r;
}

struct markov_chain *mc_new(lmpc_fid_t fid)
{
    struct markov_chain *mc = calloc(1, sizeof(*mc));
    if (!mc) return NULL;
    mc->fid = fid;
    mc->rows = malloc(MC_CAP_0 * sizeof(*mc->rows));
    if (!mc->rows) { free(mc); return NULL; }
    mc->row_cap = MC_CAP_0;
    return mc;
}

void mc_emit(struct markov_chain *mc, lmpc_region_t from, lmpc_region_t to)
{
    struct m_row *r = find_or_create(mc, from);
    for (size_t i = 0; i < r->edge_count; i++)
        if (r->edges[i].to == to) { r->edges[i].count++; return; }
    if (r->edge_count >= r->edge_cap) {
        r->edge_cap *= 2;
        r->edges = realloc(r->edges, r->edge_cap * sizeof(*r->edges));
    }
    r->edges[r->edge_count++] = (struct m_edge){to, 1};
    mc->tick++;
    if (mc->tick >= MPC_DECAY_W) mc_decay(mc);
}

int mc_predict(const struct markov_chain *mc, lmpc_region_t src,
               lmpc_region_t *targets, int max_k)
{
    const struct m_row *best = NULL;
    for (size_t i = 0; i < mc->row_n; i++)
        if (mc->rows[i].from == src) { best = &mc->rows[i]; break; }
    if (!best || !best->edge_count) return 0;

    /* Top-K greedy selection on local copy of counts */
    int ec = (int)best->edge_count;
    lmpc_region_t dest[256]; uint32_t w[256]; uint8_t used[256] = {0};
    for (int j = 0; j < ec; j++) { dest[j] = best->edges[j].to; w[j] = best->edges[j].count; }

    int nfound = 0;
    for (int p = 0; p < max_k && nfound < max_k; p++) {
        int mi = -1; uint32_t mw = 0;
        for (int j = 0; j < ec; j++)
            if (!used[j] && w[j] > mw) { mw = w[j]; mi = j; }
        if (mi < 0 || mw == 0) break;
        targets[nfound++] = dest[mi];
        used[mi] = 1;
    }
    return nfound;
}

void mc_decay(struct markov_chain *mc)
{
    for (size_t i = 0; i < mc->row_n; i++)
        for (size_t j = 0; j < mc->rows[i].edge_count; j++)
            mc->rows[i].edges[j].count /= 2;
    mc->tick = 0;
}

void mc_destroy(struct markov_chain *mc)
{
    if (!mc) return;
    for (size_t i = 0; i < mc->row_n; i++) free(mc->rows[i].edges);
    free(mc->rows);
    free(mc);
}
