/* lustre-mpc — Native Slurm BurstBuffer C plugin API */
#ifndef LUSTRE_MPC_PLUGIN_H
#define LUSTRE_MPC_PLUGIN_H
#include <stdint.h>
#include <slurm/slurm.h>
extern char plugin_type[];
extern char plugin_name[];
extern const uint32_t plugin_version;
extern int init(void);
extern void fini(void);
extern uint32_t slurm_bb_job_process(char* job_script, char** error_msg, void *userdata);
extern int slurm_bb_pools(int pool_id, size_t bb_size, char* script_info);
extern int slurm_bb_setup(int job_id, uid_t uid, gid_t gid, int pool_id, size_t bb_size, char *script_info);
extern int slurm_bb_data_in(int job_id, struct bb_job_data_in *data_in);
extern int slurm_bb_data_out(int job_id, char *job_script);
extern int slurm_bb_test_data_in(int job_id, struct bb_job_data_in *data_in);
extern int slurm_bb_real_size(int job_id, char* real_size_str, void *userdata);
extern int slurm_bb_paths(int job_id, char *job_script, void *path_file, void *userdata);
extern int slurm_bb_pre_run(int job_id, char *job_script, void *userdata);
extern int slurm_bb_post_run(int job_id, char *job_script, void *userdata);
extern int slurm_bb_job_teardown(int job_id, char *job_script, int hurry);
#endif /* LUSTRE_MPC_PLUGIN_H */
