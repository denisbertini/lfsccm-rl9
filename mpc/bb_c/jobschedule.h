/* lustre-mpc — Job-to-node routing from config */
#ifndef LUSTRE_MPC_JOBSCHED_H
#define LUSTRE_MPC_JOBSCHDULE_H
#include "common/config.h"
int find_node_idx(struct runtime_cfg *cfg, const char* hostname);
const char* get_socket_path(void);
#endif /*LUSTRE_MPC_JOBSCHED_H*/
