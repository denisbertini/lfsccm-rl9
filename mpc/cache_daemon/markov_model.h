/* lustre-mpc — First-order sparse Markov prediction engine */
#ifndef LUSTRE_MPC_MARKOV_H
#define LUSTRE_MPC_MARKOV_H
#include "common/types.h"

struct m_edge { lmpc_region_t to; uint32_t count; };

struct m_row {
    lmpc_region_t   from;
    struct m_edge  *edges;
    size_t          edge_count, edge_cap;
};

struct markov_chain {
    lmpc_fid_t     fid;
    struct m_row  *rows;
    size_t         row_n, row_cap;
    uint32_t       tick;
};

struct markov_chain *mc_new(lmpc_fid_t fid);
void mc_emit(struct markov_chain *mc, lmpc_region_t from, lmpc_region_t to);
int  mc_predict(const struct markov_chain *m, lmpc_region_t src, lmpc_region_t *targets, int max_k);
void mc_decay(struct markov_chain *mc);
void mc_destroy(struct markov_chain *mc);

#endif /* LUSTRE_MPC_MARKOV_H */
