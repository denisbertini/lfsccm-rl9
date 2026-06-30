/* lustre-mpc — Unix-domain socket IPC protocol                         */
#ifndef LUSTRE_MPC_IPC_H
#define LUSTRE_MPC_IPC_H

#include "common/types.h"

#define IPC_MAGIC       0xFFFEEE5Au
#define IPC_PATH_MAX   256
#define IPC_BACKLOG      5

#define IPC_MISS         1
#define IPC_SLOT_READY   2
#define IPC_SHUTDOWN     3

#pragma pack(push, 1)
struct ipc_hdr    { uint32_t magic;  uint16_t type; uint16_t pld_len; };
struct ipc_miss   { struct lmpc_extent ext; off_t exact_off; size_t len; char spath[IPC_PATH_MAX]; };
struct ipc_ready  { struct lmpc_extent ext; int slot_idx; };
struct ipc_shutdown{ uint32_t job_id; };
#pragma pack(pop)

/* ── daemon side ─────────────── */
int  ipc_listen(const char *socket_path, int backlog);
void ipc_close(int fd, const char *socket_path);
int  ipc_accept(int lfd, int timeout_ms);
ssize_t ipc_send_raw(int fd, const void *data, size_t len);
int  ipc_recv_hdr(int fd, struct ipc_hdr *out, int to_ms);
int  ipc_recv_body(int fd, void *buf, size_t want, int to_ms);

/* ── hook side ─────────────── */
int  ipc_connect(const char *socket_path);
int  ipc_req_fill(int fd, const struct ipc_miss *req, struct ipc_ready *resp, int to_ms);
int  ipc_notify_shutdown(int fd, uint32_t job_id);
void ipc_disconnect(int fd);

#endif /* LUSTRE_MPC_IPC_H */
