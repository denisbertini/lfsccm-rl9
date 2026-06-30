/* lustre-mpc — /tmp budgeting and directory management */
#ifndef LUSTRE_MPC_TMPMGMT_H
#define LUSTRE_MPC_TMPMGMT_H
#include <stddef.h>
#include <stdint.h>
size_t tmp_probe_usable(const char *dir);
int    tmp_slot_count_for(size_t avail_bytes);
int    tmp_mkdir_job_cache(char *out, size_t osz, const char *prefix, uint32_t jid);
int    tmp_rmdir_job_cache(const char *path);
#endif /* LUSTRE_MPC_TMPMGMT_H */
