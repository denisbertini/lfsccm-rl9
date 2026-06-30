#!/bin/bash
# Setup Lustre filesystem on loopback devices
# Idempotent: safe to re-run (will unmount existing mounts first)
set -eu

export MYIP=$(hostname -I | awk '{print $1}')
export IAM=$(whoami)

echo "=== Using IP: ${MYIP} ==="

# --- Idempotent cleanup of any prior mounts ---
for mnt in /mnt/lustre /lustre/ost1 /lustre/ost0 /lustre/mdt; do
    sudo umount -f "$mnt" 2>/dev/null || true
done
sleep 1

# --- Create loopback storage files (skip if they already exist) ---
sudo mkdir -p /srv/lustre
if [ ! -f /srv/lustre/lmdt ]; then
    echo "Creating loopback image /srv/lustre/lmdt ..."
    sudo dd if=/dev/zero of=/srv/lustre/lmdt bs=1M count=1000
fi
if [ ! -f /srv/lustre/lost0 ]; then
    echo "Creating loopback image /srv/lustre/lost0 ..."
    sudo dd if=/dev/zero of=/srv/lustre/lost0 bs=1M count=1000
fi
if [ ! -f /srv/lustre/lost1 ]; then
    echo "Creating loopback image /srv/lustre/lost1 ..."
    sudo dd if=/dev/zero of=/srv/lustre/lost1 bs=1M count=1000
fi

# --- Format Lustre targets (only if not yet formatted) ---
if ! sudo blkid /srv/lustre/lmdt | grep -q LUSTRE_lfs; then
    printf "\n[1/3] Formatting MGS+MDT...\n"
    sudo mkfs.lustre --fsname=lfs --mgs --mdt /srv/lustre/lmdt
else
    printf "\n[1/3] MGS+MDT already formatted, skipping.\n"
fi

if ! sudo blkid /srv/lustre/lost0 | grep -q LUSTRE_lfs; then
    printf "\n[2/3] Formatting OST0...\n"
    sudo mkfs.lustre --fsname=lfs --index=0 --ost --mgsnode=${MYIP}@tcp /srv/lustre/lost0
else
    printf "\n[2/3] OST0 already formatted, skipping.\n"
fi

if ! sudo blkid /srv/lustre/lost1 | grep -q LUSTRE_lfs; then
    printf "\n[3/3] Formatting OST1...\n"
    sudo mkfs.lustre --fsname=lfs --index=1 --ost --mgsnode=${MYIP}@tcp /srv/lustre/lost1
else
    printf "\n[3/3] OST1 already formatted, skipping.\n"
fi

# --- Mount server-side targets ---
sudo mkdir -p /lustre/mdt /lustre/ost0 /lustre/ost1

printf "\nMounting MDT...\n"
sudo mount -t lustre -o loop /srv/lustre/lmdt /lustre/mdt

printf "Mounting OST0...\n"
sudo mount -t lustre -o loop /srv/lustre/lost0 /lustre/ost0

printf "Mounting OST1...\n"
sudo mount -t lustre -o loop /srv/lustre/lost1 /lustre/ost1

# --- Mount client view ---
printf "\nMounting client FS...\n"
sudo mkdir -p /mnt/lustre

# Replace `mount.tlt` with iptables flush (Lustre 2.17 EL9 removed mount.tlt)
sudo systemctl stop firewalld 2>/dev/null || true
sudo iptables -F 2>/dev/null  || true

# Client mount hangs until MGS reply arrives; retry briefly
for i in 1 2 3; do
    if sudo mount -t lustre "${MYIP}@tcp:/lfs" /mnt/lustre 2>/dev/null; then
        break
    fi
    sleep 3
done

sleep 2
sudo chown "${IAM}:${IAM}" /mnt/lustre

# --- Sanity check ---
echo "lfsccm-rl9 hello" > /mnt/lustre/sample
cat /mnt/lustre/sample
lfs df /mnt/lustre

printf "\n=== Lustre server is ready ===\n"
printf "Server IP : %s\n" "${MYIP}"
printf "Client MT : /mnt/lustre\n"
