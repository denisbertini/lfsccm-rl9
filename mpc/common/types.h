/* lustre-mpc — Shared types across all modules */
#ifndef LUSTRE_MPC_TYPES_H
#define LUSTRE_MPC_TYPES_H
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#define MPC_CHUNK_SIZE     (64ULL * 1024 * 1024)
#define MPC_REGION_SHIFT   26
#define MPC_PREFETCH_DEPTH 2
#define MPC_DECAY_W        1000
#define MPC_TMP_SAFETY      0.15
#define MPC_MAX_SLOTS       1100

typedef uint64_t lmpc_fid_t;
typedef uint64_t lmpc_region_t;

static inline lmpc_region_t offset_to_region(off_t off)
{ return (lmpc_region_t)(off >> MPC_REGION_SHIFT); }

static inline off_t region_to_offset(lmpc_region_t r)
{ return (off_t)r << MPC_REGION_SHIFT; }

struct lmpc_extent {
    lmpc_fid_t    fid;
    lmpc_region_t region;
    uint32_t      gen;
};

enum lmpc_rc {
    MPC_OK          =  0,
    MPC_CACHE_HIT   =  1,
    MPC_CACHE_MISS  = -1,
    MPC_ERR_IO      = -10,
};

#endif /* LUSTRE_MPC_TYPES_H */
