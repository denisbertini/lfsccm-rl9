/* lustre-mpc — Disk space handling */
#include "cache_daemon/tmp_mgmt.h"
#include "common/types.h"
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdio.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/limits.h>

size_t tmp_probe_usable(const char *dir) {
    struct statvfs sv;
    if (statvfs(dir, &sv)) return 0;
    uint64_t free_b = (uint64_t)sv.f_bavail * (uint64_t)sv.f_frsize;
    double usable = (double)free_b * (1.0 - MPC_TMP_SAFETY);
    return (size_t)(usable > 0 ? usable : 0);
}

int tmp_slot_count_for(size_t avail) {
    if (!avail) return 0;
    int n = (int)(avail / MPC_CHUNK_SIZE);
    if ((unsigned)n > MPC_MAX_SLOTS) n = MPC_MAX_SLOTS;
    return n > 0 ? n : 1;
}

int tmp_mkdir_job_cache(char *out, size_t osz, const char *prefix, uint32_t jid)
{
    snprintf(out, osz, "%s/cache_%u/slots", prefix, jid);
    mkdir(prefix, 0750);
    mkdir(out, 0750);
    return access(out, F_OK)?-1:0;
}

int tmp_rmdir_job_cache(const char *path)
{
    char cmd[PATH_MAX+64];
    snprintf(cmd, sizeof(cmd), "rm -rf '%s'", path);
    return system(cmd)==EXIT_SUCCESS?0:-1;
}
