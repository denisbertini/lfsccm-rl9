# lfsccm-rl9

Adaptation of the [lfsccm lustre-vm](https://github.com/hpc-cloudos/lfsccm) setup from CentOS 8 (Lustre 2.14) to **Rocky Linux 9** (Lustre 2.17).

## Original Project

- **Authors:** Kota Tsuyuzaki & Yusuke Kaneko (NTT Corporation)
- **License:** MIT (DDN Storage, 2022)
- **Repo:** https://github.com/hpc-cloudos/lfsccm

## Changes

| Component | Original (lfsccm) | This repo (lfsccm‑rl9) |
|-----------|-------------------|------------------------|
| OS image  | CentOS 8.3        | Rocky Linux 9.x |
| Lustre    | 2.14              | 2.17 |
| Kernel    | Pre-built kmods   | Whitelisted kernel built into the qcow2; matches `kmod-lustre` without rebuild |
| Repo URL  | Whamcloud el8.3   | Whamcloud el9.7 |

## Prerequisites

- [Lima](https://github.com/lima-vm/lima) + QEMU with KVM enabled
- ≥ 4 GiB available RAM (≥ 6 GiB recommended for kmod tooling)

## Quick Start

Once:
```bash
git clone --recursive https://github.com/DDNStorage/fio.git
cd fio
./configure --with-libaio
make -j"$(nproc)"
sudo make install
```

Every session:
```bash
git pull --ff-only                                       # fetch latest scripts
limactl start lustre                                     # boot VM (downloads ~3 GB image on first run)
limactl shell lustre sudo bash /usr/local/bin/lustre-setup.sh
```

## What the scripts do

| Script | Action |
|--------|--------|
| `lustre/pre-install.sh` | Disables SELinux, adds Lustre/e2fsprogs/repos, updates system (excludes stock kernel). Runs only once. |
| `lustre/install.sh` | Installs `kmod-lustre`, `lustre`, and Whamcloud e2fsprogs via dnf with a temporary `--disablerepo='*'`. |
| `lustre/setup.sh` | Creates loopback targets, formats them, mounts server-side, then mounts client view at `/mnt/lustre`. Idempotent — safe to re-run. |
| `/usr/local/bin/lustre-setup.sh` | Same idempotent logic pushed to the VM. The systemd unit `lustre-mount.service` calls this script every time you do `limactl start lustre`. |

### Running workload tests (once inside the VM or over SSH)

```bash
# Run 5 iterations with 4 concurrent jobs
fio_ddn_run -o lustre-fio.yaml --count 5 --concurrency 4
```

Each iteration will output results like:
```
read: IOPS=..., BW=... MiB/s (eta=...)
write: IOPS=..., BW=... MiB/s
```

Use `--output json` or post-process via [`hpc_cloudos/lfsccm`](https://github.com/hpc-cloudos/lfsccm/) analysis scripts later.

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| Client mount hangs on startup | Firewall may block MGS reply. If needed, run `sudo iptables -F` before mounting or set `firewalld` to allow port 988/tcp (see below). |
| Cannot open display in GUI apps | Pass `DISPLAY=:0` when using Lima's X11 forwarding, or use Lima v0.23+ built-in `-x` option. Alternatively, forward `SSH_AUTH_SOCK` so the remote user can authenticate interactively. |
| Repeated bad‑mirror download failures | Refresh DNF metadata cache before installing large packages: `sudo dnf makecache --refresh`. Then retry `setup.sh` or individual steps. |

### Enabling firewalld permanently

To avoid needing the iptables fallback, open the required Lustre TCP ports manually:

```
sudo firewall-cmd --permanent --add-port=988/tcp       # LNET/ksocklnd
sudo firewall-cmd --permanent --add-port=988/udp
sudo firewall-cmd --reload
```

If you ever disable it again:
```
sudo systemctl disable --now firewalld
```
