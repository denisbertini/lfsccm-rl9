/* lustre-mpc — Native Slurm BurstBuffer C plugin entrypoint */
#include "bb_c/plugin.h"
#include "common/config.h"
#include "bb_c/jobschedule.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define _GNU_SOURCE
#include <slurm/slurm.h>

char plugin_type[] = "burst_buffer/lustre_mpc";
char plugin_name[] = "Lustre Memory-Predictive Cache";
const uint32_t plugin_version = SLURM_VERSION_NUMBER;

static struct runtime_cfg g_cfg;

int init(void) {
    if (cfg_parse_file("/etc/slurm/lfsccm.conf", &g_cfg) != 0) {
        slurm_log_error("[mpc] Cannot open config, falling back to node list");
        return -1;
    }
    slurm_log_info("[mpc] Loaded %zu nodes from config", g_cfg.n_nodes);
    return SLURM_SUCCESS;
}

void fini(void) {
    cfg_destroy(&g_cfg);
}

uint32_t slurm_bb_job_process(char *job_script, char **error_msg, void *userdata) {
    slurm_log_info("[mpc] job_process script=%s", job_script ? job_script : "(null)");
    /* Validate #MPC directives in script later */
    return SLURM_SUCCESS;
}

int slurm_bb_pools(int pool_id, size_t bb_size, char *script_info) {
    slurm_log_info("[mpc] pools id=%d sz=%zu", pool_id, (unsigned long)bb_size);
    return SLURM_SUCCESS;
}

int slurm_bb_setup(int job_id, uid_t uid, gid_t gid, int pool_id, size_t bb_size, char *script_info) {
    slurm_log_info("[mpc] setup jid=%d uid=%u gid=%u pool=%d sz=%zu",
           job_id, (unsigned)uid, (unsigned)gid, pool_id, (unsigned long)bb_size);
    return SLURM_SUCCESS;
}

int slurm_bb_data_in(int job_id, struct bb_job_data_in *data_in) {
    slurm_log_info("[mpc] data_in jid=%d files=", job_id);
    for (size_t i = 0; i < g_cfg.n_nodes; i++) {
        slurm_log_info("[mpc]   node %s will get cache_daemon", g_cfg.nodes[i].name);
    }
    return SLURM_SUCCESS;
}

int slurm_bb_test_data_in(int job_id, struct bb_job_data_in *data_in) {
    slurm_log_info("[mpc] test_data_in jid=%d", job_id);
    return SLURM_SUCCESS;
}

int slurm_bb_real_size(int job_id, char **real_size_str, void *userdata) {
    slurm_log_info("[mpc] real_size jid=%d", job_id);
    return SLURM_SUCCESS;
}

int slurm_bb_paths(int job_id, char *job_script, char *path_file, void *userdata) {
    slurm_log_info("[mpc] paths jid=%d pathfile=%s", job_id, path_file);
    FILE *fp = fopen(path_file, "a");
    if (!fp) {
        slurm_log_error("[mpc] Cannot write env file %s", path_file);
        return SLURM_ERROR;
    }
    fprintf(fp, "export LD_PRELOAD=/usr/local/lib/libpreadv_hook.so\n");
    fclose(fp);
    return SLURM_SUCCESS;
}

int slurm_bb_pre_run(int job_id, char *job_script, void *env) {
    slurm_log_info("[mpc] pre_run jid=%d", job_id);
    return SLURM_SUCCESS;
}

int slurm_bb_post_run(int job_id, char *job_script, void *env) {
    slurm_log_info("[mpc] post_run jid=%d", job_id);
    return SLURM_SUCCESS;
}

int slurm_bb_data_out(int job_id, struct bb_job_data_out *data_out) {
    slurm_log_info("[mpc] data_out jid=%d", job_id);
    return SLURM_SUCCESS;
}

int slurm_bb_job_teardown(int job_id, char *job_script, int hurry, void *userdata) {
    slurm_log_info("[mpc] teardown jid=%d hurry=%d", job_id, hurry);
    return SLURM_SUCCESS;
}
