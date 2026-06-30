/* lustre-mpc — Native Slurm BurstBuffer C plugin API */
#ifndef LUSTRE_MPC_PLUGIN_H
#define LUSTRE_MPC_PLUGIN_H
#include <stdint.h>
#include <sys/types.h>
extern char plugin_type[];
extern char plugin_name[];
extern const uint32_t plugin_version;
extern int init(void);
extern void fini(void);
#endif /* LUSTRE_MPC_PLUGIN_H */
