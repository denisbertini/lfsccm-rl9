#!/bin/bash
set -euo pipefail
# Boot Lima, install Lustre on Rocky 9 inside VM, mount /mnt/lustre.
# Files delivered via rsync (no host-mount).
HOST_DIR="/home/denis/work/ddn/lfsccm-rl9"
VM_NAME="lustre"
REPO="/opt/lfsccm-rl9"
YAML="$HOST_DIR/lustre.yaml"

_sync() {
    local SOCK="/home/denis/.lima/${VM_NAME}/ssh.sock"
    # Wait for SSH socket ready — up to 15 sec after boot/restart
    while [ ! -S "$SOCK" ]; do sleep 1; done
    ssh -S "$SOCK" lima-lookup@localhost "sudo mkdir -p ${REPO} && sudo chown denis:denis ${REPO}"
    rsync -a --delete -e "ssh -S $SOCK" "$HOST_DIR/" lima-lookup@localhost:"${REPO}/"
}

echo "=== [1/6] BOOT Lima VM ==="
limactl start "$YAML"
sleep 5 && _sync

echo "=== [2/6] PRE-INSTALL (repos, SELinux) ==="
limactl shell "$VM_NAME" bash -c "bash $REPO/lustre/pre-install.sh"

echo "=== [3/6] REBOOT → install Docker CE ==="
limactl restart "$VM_NAME"
sleep 5 && _sync
limactl shell "$VM_NAME" sudo bash -c 'dnf install -y --setopt=_skip_missing_digests=True docker-ce* containerd.io &\systemctl enable --now docker'

echo "=== [4/6] INSTALL Lustre server + prepare initramfs ==="
limactl shell "$VM_NAME" bash -c "bash $REPO/lustre/install.sh"

echo "=== [5/6] REBOOT → format/mount Lustre ==="
limactl restart "$VM_NAME"
sleep 5 && _sync
limactl shell "$VM_NAME" bash -c "bash $REPO/lustre/setup.sh"

echo ""; echo "=============================================="
echo "    lustre is ready at /mnt/lustre"
echo " Enter the VM: limactl shell lustre"
echo " Start Slurm-Docker:"
echo "   limactl shell lustre bash -c 'cd $REPO/slurm-docker &\ sudo docker compose up -d'"