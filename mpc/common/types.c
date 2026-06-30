/* lustre-mpc — Common helpers                                   */
#include "common/types.h"
#include <sys/stat.h>
#include <stdio.h>

lmpc_fid_t derive_fid(const char *path)
{
    struct stat sb;
    if (stat(path, &sb)) return 0;
    return (lmpc_fid_t)sb.st_ino | ((lmpc_fid_t)sb.st_dev << 32);
}

const char *rc_strerror(enum lmpc_rc rc)
{
    switch (rc) {
        case MPC_OK:          return "OK";
        case MPC_CACHE_HIT:   return "HIT";
        case MPC_CACHE_MISS:  return "MISS";
        case MPC_ERR_IO:      return "IO";
        default:              return "?";
    }
}
