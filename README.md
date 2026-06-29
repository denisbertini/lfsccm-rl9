# lfsccm-rl9

Adaptation of the [lfsccm lustre-vm](https://github.com/hpc-cloudos/lfsccm) setup from CentOS 8 (Lustre 2.14) to **Rocky Linux 9.7** (Lustre 2.17).

## Original Project

- **Authors:** Kota Tsuyuzaki & Yusuke Kaneko (NTT Corporation)
- **License:** MIT (DDN Storage, 2022)
- **Repo:** https://github.com/hpc-cloudos/lfsccm

## Changes

| Component | Original (lfsccm) | This repo (lfsccm‑rl9) |
|-----------|-------------------|------------------------|
| OS image  | CentOS 8.3        | Rocky Linux 9.7 |
| Lustre    | 2.14              | 2.17 |
| Packages  | `kmod-lustre` pre-built for EL8 kernel | `kmodtool prepare` to recompile against RL9 default kernel |
| Repo URL  | Whamcloud el8.3   | Whamcloud el9.7 |

## Quick Start

```bash
limactl start lustre-rl9                          # boot VM + download image
limactl shell lustre-rl9 lustre/pre-install.sh    # add repos
limactl stop lustre-rl9 && limactl start lustre-rl9
limactl shell lustre-rl9 lustre/install.sh        # RPMs + kmodtool recompile
limactl stop lustre-rl9 && limactl start lustre-rl9
limactl shell lustre-rl9 lustre/setup.sh          # format & mount MDT/OST
```

See `lustre/*.sh` scripts for details.
