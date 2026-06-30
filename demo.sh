#!/bin/bash
set -euo pipefail

VM_NAME="lustre"
REPO="/home/denis/work/ddn/lfsccm-rl9"

echo "=== [Step 1/5] Boot Lima VM ==="
limactl start "$VM_NAME" || true

echo "=== [Step 2/5] Pre-install (repos, SELinux, Docker Engine) ==="
limactl shell "$VM_NAME" bash -c "bash $REPO/lustre/pre-install.sh"

echo "=== [Step 3/5] Reboot for package deps ==="
limactl stop "$VM_NAME" && limactl start "$VM_NAME"

echo "=== [Step 4/5] Install Lustre packages + kmodtool ==="
limactl shell "$VM_NAME" bash -c "bash $REPO/lustre/install.sh"

echo "=== [Step 5/5] Reboot + format/mount Lustre ==="
limactl stop "$VM_NAME" && limactl start "$VM_NAME"
limactl shell "$VM_NAME" bash -c "bash $REPO/lustre/setup.sh"

echo ""
echo "=============================================="
echo "  Lustre is ready at /mnt/lustre"
echo "  Enter the VM:  limactl shell lustre"
echo "  Start Slurm-Docker:"
echo "    limactl shell lustre bash -c 'cd $REPO/slurm-docker && docker-compose up'"
echo "=============================================="
