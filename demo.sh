#!/bin/bash
set -euo pipefail

echo "=== Step 1: Boot Lima VM ==="
limactl start lustre || true

echo "=== Step 2: Lustpre pre-install (repol install) ==="
limactl shell lustre bash -c "bash /home/$(whoami)/work/ddn/lfsccm-rl9/lustre/pre-install.sh"

echo "=== Step 3: Reboot for kernel deps ==="
limactl stop lustre && limactl start lustre

echo "=== Step 4: Install Lustre packages + kmodtool ==="
limactl shell lustre bash -c "bash /home/$(whoami)/work/ddn/lfsccm-rl9/lustre/install.sh"

echo "=== Step 5: Format & mount Lustre ==="
limactl shell lustre bash -c "bash /home/$(whoami)/work/ddn/lfsccm-rl9/lustre/setup.sh"

echo "=== Done! Enter VM:   limactl shell lustre"