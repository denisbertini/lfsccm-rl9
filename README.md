# lfsccm-rl9

Adaptation of the [lfsccm lustre-vm](https://github.com/hpc-cloudos/lfsccm) setup from CentOS 8 (Lustre 2.14) to **Rocky Linux 9** (Lustre 2.17).

## Overview

`lfsccm` is the open source software project to support burst buffer feature for [Lustre Filesystem](https://www.lustre.org/) in [Slurm Workload Manager](https://slurm.schedmd.com/). This `lfsccm` depends on the newer features of Slurm and Lustre, called Slurm BurstBuffer Lua generics and Lustre Persistent Client Cache.

This repository provides a generalized multi-node test environment using Docker Compose running inside a Lima VM, with both legacy PCC (`burst_buffer/lua`) and native C plug-in (`lustre_mpc`) implementations available for comparison.

This software is released under the MIT License, see LICENSE file in this repository.

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
| Test harness | Single slurmctld + slurmd in VM | Docker Compose multi-node cluster inside Lima VM |
| BurstBuffer | Lua + Python only | Native C plugin (`lustre_mpc`) alongside legacy Lua |

## Prerequisites

- [Lima](https://github.com/lima-vm/lima) + QEMU with KVM enabled
- ≥ 4 GiB available RAM (≥ 6 GiB recommended for kmod tooling)

## Quick Start

```bash
limactl start lustre                                     # boot VM (downloads ~3 GB image on first run)
limactl shell lustre sudo bash /usr/local/bin/lustre-setup.sh  # idempotent Lustre bringup
```

After Lustre is ready, launch Slurm-Docker inside the VM:

```bash
limactl shell lustre bash -c 'cd /home/denis/work/ddn/lfsccm-rl9/slurm-docker && docker-compose up -d'
```

Then interact normally:

```bash
limactl shell lustre sinfo
limactl shell lustre sbatch my_job.sh
limactl shell lustre squeue
```

## What the Scripts Do

| Script | Action |
|--------|--------|
| `lustre/pre-install.sh` | Disables SELinux, adds Lustre repos, updates system (excludes stock kernel). Runs once. |
| `lustre/install.sh` | Installs `kmod-lustre`, `lustre`, and Whamcloud e2fsprogs via dnf. |
| `lustre/setup.sh` | Creates loopback targets, formats them, mounts server-side, then mounts client view at `/mnt/lustre`. Idempotent — safe to re-run. |

## Project Structure

```
lfsccm-rl9/
├── lfsccm/                   # Legacy PCC toolchain (Lua + Python CLI)
│   ├── bb_lua/burst_buffer.lua
│   └── main.py               # lfsccm attach/detach/check commands
├── mpc/                      # lustre-mpc native C BurstBuffer plugin
│   ├── CMakeLists.txt
│   ├── bb_c/plugin.c         # Slurm BB Callbacks
│   ├── common/               # Types, config parsing, IPC protocol
│   ├── cache_daemon/         # Ring buf, Markov prefetch
│   └── preadv_hook/hook.c    # LD_PRELOAD intercept
├── slurm-docker/             # Docker Compose stack
│   ├── Dockerfile            # Builds Slurm + MPC plugin
│   ├── docker-compose.yml    # Base (Lustre-free, works standalone)
│   ├── docker-compose.lustre.yml  # Adds /mnt/lustre:/lustre bind mounts
│   ├── docker-entrypoint.sh  # Entry script (BB_PLUGIN env switch)
│   ├── slurm.conf
│   └── cgroup.conf
├── lustre.yaml               # Lima VM config (Rocky 9)
├── lustre/                   # Idempotent Lustre bringup scripts
└── demo.sh                   # Full automated bootstrap
```

## BurstBuffer Plugin Comparison

Two plugins coexist, swappable via `BB_PLUGIN` environment variable in entrypoint:

| Mode | Directive | Mechanism | Activation |
|------|-----------|-----------|------------|
| Legacy PCC | `#PCC --path=... --mode=ro` | Whole-file pre-copy (`lfs pcc attach`) | `BB_PLUGIN=lua` (default) |
| lustre-mpc | `#MPC --path=... --mode=r` | Extent-level cache daemon + Markov prefetch | `BB_PLUGIN=mpc` |

## Development

`lfsccm` package uses python nose2 test suite:

```bash
git clone https://github.com/DDNStorage/lfsccm.git
cd lfsccm/lfsccm
pip install -r requirements.txt
pip install -r test-requirements.txt
python -m nose2
```

For the C plugin (`lustre-mpc`):

```bash
cd lfsccm-rl9/mpc
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Appendix

### Build Slurm Supporting BurstBuffer Lua

See `lustre-vm/slurm/install.sh` for reference.

### Running Workload Tests

```bash
fio_ddn_run -o lustre-fio.yaml --count 5 --concurrency 4
```

Use `--output json` or post-process via [`hpc_cloudos/lfsccm`](https://github.com/hpc-cloudos/lfsccm/) analysis scripts later.

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| Client mount hangs on startup | Firewall may block MGS reply. Run `sudo iptables -F` before mounting or set `firewalld` to allow port 988/tcp. |
| Repeated bad-mirror failures | Refresh DNF metadata: `sudo dnf makecache --refresh`. Then retry `setup.sh`. |

### Enabling firewalld permanently

```
sudo firewall-cmd --permanent --add-port=988/tcp       # LNET/ksocklnd
sudo firewall-cmd --permanent --add-port=988/udp
sudo firewall-cmd --reload
```
