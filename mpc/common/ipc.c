/* lustre-mpc — IPC via Unix-domain sockets                            */
#include "common/ipc.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <sys/stat.h>

static ssize_t xread_full(int fd, void *buf, size_t len)
{
    size_t got = 0;
    while (got < len) {
        ssize_t n = read(fd, (char*)buf + got, len - got);
        if (n <= 0) return -(ssize_t)(got ? 1 : 0);
        got += (size_t)n;
    }
    return (ssize_t)len;
}

static int wait_readable(int fd, int timeout_ms)
{
    struct pollfd p[1] = {{.fd =  fd, .events = POLLIN}};
    int r = poll(p, 1, timeout_ms);
    if (r == 0) return -1;                /* timed out            */
    return (r > 0 && p[0].revents & POLLIN) ? 0 : -1;
}

ssize_t ipc_send_raw(int fd, const void *data, size_t len)
{
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(fd, (const char*)data + sent, len - sent);
        if (n <= 0) return -(ssize_t)sent;
        sent += (size_t)n;
    }
    return (ssize_t)len;
}

/* ════ daemon side ═══════════ */

int ipc_listen(const char *socket_path, int backlog)
{
    unlink(socket_path);
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", socket_path);
    if (bind(fd, (struct sockaddr*)&addr, SUN_LEN(&addr)) != 0) goto fail;
    if (listen(fd, backlog > 0 ? backlog : IPC_BACKLOG) != 0) goto fail;
    chmod(socket_path, 0777);
    return fd;
fail:
    close(fd);
    unlink(socket_path);
    return -1;
}

void ipc_close(int fd, const char *socket_path)
{
    shutdown(fd, SHUT_RDWR);
    close(fd);
    unlink(socket_path);
}

int ipc_accept(int lfd, int to_ms)
{
    if (to_ms >= 0) {
        struct pollfd p[1] = {{.fd=lfd, .events=POLLIN}};
        int r = poll(p, 1, to_ms);
        if (r<=0) return -1;
    }
    return accept(lfd, NULL, NULL);
}

int ipc_recv_hdr(int fd, struct ipc_hdr *out, int to_ms)
{
    if (wait_readable(fd, to_ms))              return -1;
    if (xread_full(fd, out, sizeof(*out))<0)   return -1;
    if (out->magic != IPC_MAGIC)               return -2;  /* bad magic */
    return 0;
}

int ipc_recv_body(int fd, void *buf, size_t want, int to_ms)
{
    if (!want) return 0;
    if (wait_readable(fd, to_ms))             return -1;
    return (xread_full(fd, buf, want)>0)?0:-1;
}

/* ════ hook side ══════════════════ */

int ipc_connect(const char *socket_path)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd<0) return -1;
    struct sockaddr_un sa = {.sun_family = AF_UNIX};
    snprintf(sa.sun_path, sizeof(sa.sun_path), "%s", socket_path);
    if (connect(fd, (struct sockaddr*)&sa, SUN_LEN(&sa))) { close(fd); return -1; }
    return fd;
}

int ipc_req_fill(int fd, const struct ipc_miss *req, struct ipc_ready *resp, int to_ms)
{
    /* Send request */
    struct ipc_hdr hdr_out = {IPC_MAGIC, IPC_MISS, (uint16_t)sizeof(*req)};
    if (ipc_send_raw(fd, &hdr_out, sizeof(hdr_out))<0)                  return -1;
    if (ipc_send_raw(fd, req,          sizeof(*req))<0)                  return -1;

    /* Receive response */
    struct ipc_hdr hdr_in;
    if (ipc_recv_hdr(fd, &hdr_in, to_ms))                                return -1;
    if (hdr_in.type!=IPC_SLOT_READY || hdr_in.pld_len!=(uint16_t)sizeof(*resp))
        return -1;
    if (ipc_recv_body(fd, resp, sizeof(*resp), to_ms))                   return -1;
    return 0;
}

int ipc_notify_shutdown(int fd, uint32_t job_id)
{
    struct ipc_shutdown sh = {.job_id = job_id};
    struct ipc_hdr h = {IPC_MAGIC, IPC_SHUTDOWN, (uint16_t)sizeof(sh)};
    if (ipc_send_raw(fd, &h, sizeof(h))<0)            return -1;
    return (ipc_send_raw(fd, &sh, sizeof(sh))>0)?0:-1;
}

void ipc_disconnect(int fd) { shutdown(fd, SHUT_RDWR); close(fd); }
