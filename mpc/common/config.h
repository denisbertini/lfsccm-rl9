/* lustre-mpc — Configuration parsing from lfsccm.conf format            */
#ifndef LUSTRE_MPC_CONFIG_H
#define LUSTRE_MPC_CONFIG_H

#include "common/types.h"

struct node_cfg { char name[256]; int rw_id, ro_id; };

struct runtime_cfg {
    struct node_cfg   *nodes;
    size_t             n_nodes;
    const char        *cache_prefix;    /* e.g. "/tmp/mempc_<jobid>"       */
    size_t             usable_bytes;    /* after safety margin               */
    int                slot_count;      /* derived from usable / CHUNK_SIZE */
};

int  cfg_parse_file(const char *path, struct runtime_cfg *cfg);
int  cfg_probe_budget(struct runtime_cfg *cfg);
void cfg_destroy(struct runtime_cfg *cfg);

#endif /* LUSTRE_MPC_CONFIG_H */
