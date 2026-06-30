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
        slurm_log_error("[bb_lustre_mpc] Cannot parse lfsccm.conf");
        return -1;
    }
    slurm_log_info("[bb_lustre_mpc] Loaded %zu nodes from config", g_cfg.n_nodes);
    return SLURM_SUCCESS;
}
void fini(void) {
    cfg_destroy(&g_cfg);
}
uint32_t slurm_bb_job_process(char* job_script, char** error_msg, void *userdata) {
    slurm_log_info("[bb_lustre_mpc] job_process script=%s", job_script ? job_script : "(null)");
    /* Basic validation will be added later */
    return SLURM_SUCCESS;
}
int slurm_bb_pools(int pool_id, size_t bb_size, char* script_info) {
    slurm_log_info("[bb_lustre_mpc] pools id=%d sz=%zu", pool_id, bb_size);
    return SLURM_SUCCESS;
}
int slurm_bb_setup(int job_id, uid_t uid, gid_t gid, int pool_id, size_t bb_size, char* script_info) {
    slurm_log_info("[bb_lustre_mpc] setup jid=%d uid=%u gid=%u pool=%d sz=%zu", job_id, (unsigned)uid, (unsigned)gid, pool_id, bb_size);
    return SLURM_SUCCESS;
}
int slurm_bb_data_in(int job_id, struct bb_job_data_in *data_in) {
    slurm_log_info("[bb_lustre_mpc] data_in jid=%d", job_id);
    for (size_t i = 0; i < g_cfg.n_nodes; i++) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "/usr/local/bin/cache_daemon /tmp/mpc_%d /lustre/data.sock socket:%s@tcp:/lfs %u", job_id, get_socket_path());
        slurm_log_info("[bb_lustre_mpc] Spawning daemon on node %s: %s", g_cfg.nodes[i].name, cmd);
    }
    return SLURM_SUCCESS;
}
int slurm_bb_test_data_in(int job_id, struct bb_job_data_in *data_in) {
    return SLURM_SUCCESS;
}
int slurm_bb_real_size(int job_id, char* real_size_str, void *userdata) {
    slurm_log_info("[bb_lustre_mpc] real_size jid=%d", job_id);
    return SLURM_SUCCESS;
}
int slurm_bb_paths(int job_id, char *job_script, char *path_file, void *userdata) {
    FILE *fp = fopen(path_file, "a");
    if (fp) {
        fprintf(fp, "export LD_PRELOAD=/usr/local/lib/libpreadv_hook.so\n");
        fclose(fp);
    }
    return SLURM_SUCCESS;
}
int slurm_bb_pre_run(int job_id, char *job_script, void *userdata) {
    slurm_log_info("[bb_lustre_mpc] pre_run jid=%d", job_id);
    return SLURM_SUCCESS;
}
int slurm_bb_post_run(int job_id, char *job_script, void *userdata) {
    slurm_log_info("[bb_lustre_mpc] post_run jid=%d", job_id);
    return SLURM_SUCCESS;
}
int slurm_bb_data_out(int job_id, char *job_script) {
    slurm_log_info("[bb_lustre_mpc] data_out jid=%d", job_id);
    return SLURM_SUCCESS;
}
int slurm_bb_job_teardown(int job_id, char *jscrip, int hurry) {
    slurm_log_info("[bb_lustre_mpc] teardown jid=%d", job_id);
    return SLURM_SUCCESS;
}
int slurm_bb_get_status(...) {
    return SLURM_SUCCESS;
}
