/* lustre-mpc — Config file parser + disk budgeting                     */
#include "common/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <errno.h>

static const char DEF_PATH[] = "/etc/slurm/lfsccm.conf";

int cfg_parse_file(const char *path, struct runtime_cfg *cfg)
{
    FILE *fp = fopen(path ? path : DEF_PATH, "r");
    if (!fp) { errno = ENOENT; return -1; }
    memset(cfg, 0, sizeof(*cfg));
    size_t cap = 8;
    cfg->nodes = calloc(cap, sizeof(*cfg->nodes));
    char buf[2048];
    while (fgets(buf, sizeof(buf), fp)) {
        if (buf[0] == '#' || buf[0] == '\n') continue;
        if (cfg->n_nodes >= cap) {
            cap *= 2;
            cfg->nodes = realloc(cfg->nodes, cap * sizeof(*cfg->nodes));
        }
        struct node_cfg *nc = &cfg->nodes[cfg->n_nodes++];
        memset(nc, 0, sizeof(*nc));
        nc->rw_id = -1; nc->ro_id = -1;
        char *tok = strtok(buf, " \t\n\r");
        while (tok) {
            if      (strncmp(tok, "NodeName=", 9) == 0) snprintf(nc->name ,sizeof(nc->name), "%s", tok+9);
            else if (strncmp(tok, "rwid=", 5)   == 0) nc->rw_id = atoi(tok+5);
            else if (strncmp(tok, "roid=", 5)   == 0) nc->ro_id = atoi(tok+5);
            tok = strtok(NULL, " \t\n\r");
        }
    }
    fclose(fp);
    return 0;
}

int cfg_probe_budget(struct runtime_cfg *cfg)
{
    const char *dir = cfg->cache_prefix ?: "/tmp";
    struct statvfs sv;
    if (statvfs(dir, &sv) != 0) return -1;
    uint64_t free_b = (uint64_t)sv.f_bavail * (uint64_t)sv.f_frsize;
    cfg->usable_bytes = (size_t)((double)free_b * (1.0 - MPC_TMP_SAFETY));
    int slots = (int)(cfg->usable_bytes / MPC_CHUNK_SIZE);
    if ((unsigned)slots > MPC_MAX_SLOTS) slots = MPC_MAX_SLOTS;
    if (slots < 1) slots = 1;
    cfg->slot_count = slots;
    return 0;
}

void cfg_destroy(struct runtime_cfg *cfg)
{
    if (!cfg) return;
    free(cfg->nodes);
    memset(cfg, 0, sizeof(*cfg));
}
