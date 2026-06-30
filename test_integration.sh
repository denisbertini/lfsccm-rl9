#!/bin/bash
set -euo pipefail

VM="lustre"
REPO="/home/denis/work/ddn/lfsccm-rl9"

echo "=== Idempotent Verification Loop ==="

for i in 1 2 3; do
    echo "--- Iteration $i: Destroy & rebuild Lima VM ---"
    
    limactl stop "$VM" 2>/dev/null || true
    limactl delete --force "$VM" 2>/dev/null || true
    
    echo "[Step 1/5] Boot Lima VM"
    limactl start "$VM" || continue
    
    echo "[Step 2/5] Pre-install (repos, Docker Engine)"
    limactl shell "$VM" bash -c "bash $REPO/lustre/pre-install.sh"
    
    echo "[Step 3/5] Reboot for deps"
    limactl stop "$VM" && limactl start "$VM"
    
    echo "[Step 4/5] Install Lustre packages"
    limactl shell "$VM" bash -c "bash $REPO/lustre/install.sh"
    
    echo "[Step 5/5] Format & mount Lustre"
    limactl stop "$VM" && limactl start "$VM"
    limactl shell "$VM" bash -c "bash $REPO/lustre/setup.sh"
    
    echo "--- Verify inside VM ---"
    limactl shell "$VM" bash -c 'df /mnt/lustro | grep lustre' || { 
        echo "ERROR: /mnt/lustre not mounted!" 
        break 
    }
done

echo "=== Done! ==="
