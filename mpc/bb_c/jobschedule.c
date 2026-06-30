/* lustre-mpc — Job-to-node scheduling helper */
#include "bb_c/jobschedule.h"
#include <string.h>

static const char socket_path[256] = "/tmp/mpc_daemon.sock";

int find_node_idx(struct runtime_cfg *cfg, const char* hostname)
{
    for (size_t i = 0; i < cfg->n_nodes; i++) {
        if (strcmp(cfg->nodes[i].name, hostname) == 0)
            return (int)i;
    }
    return -1;
}

const char* get_socket_path(void)
{
    return socket_path;
}
