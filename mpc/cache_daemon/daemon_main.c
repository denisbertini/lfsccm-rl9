/* lustre-mpc — Per-node cache daemon entrypoint */
#include "common/types.h"
#include "common/config.h"
#include "cache_daemon/tmp_mgmt.h"
#include "cache_daemon/ring_buffer.h"
#include "cache_daemon/markov_model.h"
#include "cache_daemon/prefetch_worker.h"
#include "common/ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

static volatile int g_shutdown = 0;
static struct ring_buf *g_rb = NULL;

static void handle_sigterm(int sig) { (void)sig; g_shutdown = 1; }

int main(int argc, char **argv)
{
    if (argc < 4){
        fprintf(stderr, "usage: %s <cache_dir> <lustre_file> <uds_path>\n", argv[0]);
        return 1;
    }
    const char *cdir   = argv[1];
    const char *lustre_f = argv[2];
    const char *uds    = argv[3];

    signal(SIGTERM, handle_sigterm);
    signal(SIGINT,  handle_sigterm);

    size_t usable  = tmp_probe_usable(cdir);
    int    scnt    = tmp_slot_count_for(usable);
    fprintf(stderr, "[daemon] %d slots (%zu MB total)\n",
            scnt, (usable/(1024*1024)));

    g_rb = rb_new(scnt, cdir);
    if (!g_rb) { perror("rb_new"); return 1; }

    struct pf_pool g_pf;
    pf_init(&g_pf, lustre_f, g_rb, PF_WORKERS_MAX);

    int lfd = ipc_listen(uds, IPC_BACKLOG);
    if (lfd < 0) { perror("ipc_listen"); return 1; }
    fprintf(stdout,"[daemon] listening on %s\n", uds); fflush(stdout);

    while (!g_shutdown) {
        int cfd = ipc_accept(lfd, -1);  /* block forever waiting   */
        if (cfd >= 0) {
            /* process one IPC from preload hook client */
            close(cfd);  /* placeholder: actual message handling goes here */
        }
    }

    /* cleanup */
    pf_shutdown_and_join(&g_pf);
    rb_free(g_rb);
    ipc_close(lfd, uds);
    return EXIT_SUCCESS;
}
