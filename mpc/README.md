# lustre-mpc — Lustre Memory-Predictive Cache

Native Slurm BurstBuffer C plugin that replaces the legacy `lfsccm` Lua + Python stack with an adaptive, online-learning cache daemon for HPC workloads with heavy random I/O over Lustre.

## Problem PCC Solves Wrongly

Lustre Persistent Client Cache (`lfs pcc attach`) copies entire files onto every node's local disk before the job starts. This breaks when:

* The dataset does not fit on each node alone (2.8 TB dataset > 1.4 TB /node)
* The job does random-access patterns (pre-copying everything just moves the bottleneck before execution)
* Multiple nodes duplicate identical data (wasted network traffic during staging)

The old `lfsccm` approach blocks ~47 minutes copying your full dataset to every node, then reads randomly off Lustre anyway for misses. **Worse than no cache.**

## lustre-mpc Principles

**1. Extent-Level Granularity** Instead of all-or-nothing whole-file caching, files are virtualized into fixed 64 MB slots. Only working-set extents live locally. Total cache space = sum of slot capacities across all nodes, not per-node duplication.

**2. Adaptive Markov Prefetch** A first-order sparse Markov chain observes access transitions online. After N observations per source region, it ranks likely destination regions by probability. Top-K predictions fill the K-ahead buffer slots automatically. Sequential accesses follow naturally as degenerate Markov chains (P(next|current)=1.0). Truly random access falls back to demand-fetch only—no speculative waste.

**3. Ring Buffer with LRU Eviction** Fixed-size slot ring wraps around on capacity exhaustion. Oldest untouched slot is victim. Disk usage never exceeds `/tmp` budget (15 % safety margin enforced at startup). The daemon owns its own cleanup; it explicitly bulk-unlinks all slots on job teardown via SIGTERM.

**4. Separate I/O Threads** K=2 prefetch worker threads use `io_uring` poll-mode for concurrent Lustre fetches. Zero system-call-per-operation overhead on the submit side. Workers run isolated from compute—job `pread()` hits against local `/tmp` slots serve at native NVMe speed (~10-50 μs vs hundreds of ms over Lustre network), completely bypassing MDS/OSD contention.

**5. LD_PRELOAD Interception** No FUSE overhead (FUSE kills 30-60 % bandwidth via two context-switches per syscall and zero VFS page-cache participation). The hook directly intercepts `pread()`/`preadv()` in-process, checks cache locality O(log N), serves hits from mmap'd slot memory, and signals daemon fills only on cold misses within the same call frame.

## Architecture

```
┌────────────── job process ──────────────┐
│                                        │
│   pread(fd, buf, count, offset)        │
│         │                              │
│    LD_PRELOAD → libpreadv_hook.so       │
│         │ hit? → memcpy from /tmp      │
│         │                ↘ miss → UDS RPC to daemon          │
│     record_transition(markov_model, prev_reg→cur_reg)     │
└──────────────────────────────────────┘         │
                                                 ▼
                               ┌── daemon (per-node) ──┐
                               │                       │
                               │  ring_buffer[slots]   │← /tmp/cache_<jid>/slots/slot_N
                               │                       │
                               │  markov_chain.predict │→ rank top-K next_regions
                               │                       │
                               │  io_uring workers (×K)│→ fetch Lustre chunks asynchronously
                               └─────────────────────────────────────┘

┌──────────────── slurmctld ───────────────────┐
│                                             │
│   bb_c/plugin.c:                            │
│     slurm_bb_job_process() parses #MPC directives       │
│     slurm_bb_data_in() deploys daemon to each node      │
│                                              │
└───────────────────────────────────────────────┘
```

## Build

```bash
cd lustre_mpc
mkdir build && cd build
cmake ..                          # detects liburing auto-disables Slurm plugin if headers missing
make -j $(nproc)

# Artifacts produced:
#   build/cache_daemon          — per-node background supervisor
#   build/preadv_hook.so        — LD_PRELOAD interceptor
#   build/liblmpc_*.a           — static sub-libraries
```

Install:

```bash
sudo make install          # places binaries under /usr/local/{bin,lib}
```

## Config Format

Same `lfsccm.conf` line format, kept compatible:

```ini
# lfsccm.conf  (/etc/slurm/)
NodeName=c1 rwid=1 roid=1
NodeName=c2 rwid=2 roid=2
```

Job script uses `#MPC` directive replacing legacy `#PCC`:

```bash
#!/bin/bash
#SBATCH --nodes=2
#MPC --path=/lustre/data/training.hdf5 --mode=r --recursive
./my_io_bound_app
```

Options inherited from `lfsccm`:

| Flag | Meaning |
|------|---------|
| `--path` / `-p` | File or glob pattern to cache |
| `--mode` / `-m`  | `rw` (single write-master) or `ro` (all-readers)
| `--recursive` / `-r` | Glob-expand directories with wildcards |

## When to Use

| Scenario | Suitable |
|----------|----------|
| Working set fits per-node /tmp (after extent-level deduplication) | Yes |
| Repeated iterations over same data (ML training epochs, iterative solvers) | Excellent — Markov model warms by iteration 2-3 |
| Graph traversals with hidden locality | Good — sparse adjacency captures jump topology |
| Entropy-driven RNG-seeded sampling (different every run) | Limited — demand-only fall-through applies |
| Full sequential read (one-pass scan) | Overkill — let kernel read-behind handle via `lctl param tune` instead |

## License

MIT. See LICENSE file.

Original concept inspired by NTT/NTT Communications `lfsccm`. This rewrite abandons Lua+Python entirely for native C performance and predictive I/O scheduling.
