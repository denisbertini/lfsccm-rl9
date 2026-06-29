#!/bin/bash
# Setup Lustre filesystem on loopback devices
# FIXME: this script is not idempotent — run teardown.sh first if re-creating
set -eu

export MYIP=$(hostname -I | awk '{print $1}')
export IAM=$(whoami)

echo "=== Using IP: ${MYIP} ==="

# Create loopback storage files
sudo mkdir -p /srv/lustre
sudo dd if=/dev/zero of=/srv/lustre/lmdt bs=1M count=1000
sudo dd if=/dev/zero of=/srv/lustre/lost0 bs=1M count=1000
sudo dd if=/dev/zero of=/srv/lustre/lost1 bs=1M count=1000

# Format Lustre targets
printf "\n[1/3] Formatting MGS+MDT...\n"
sudo mkfs.lustre --fsname=lfs --mgs --mdt /srv/lustre/lmdt

printf "\n[2/3] Formatting OST0...\n"
sudo mkfs.lustre --fsname=lfs --index=0 --ost --mgsnode=${MYIP}@tcp /srv/lustre/lost0

printf "\n[3/3] Formatting OST1...\n"
sudo mkfs.lustre --fsname=lfs --index=1 --ost --mgsnode=${MYIP}@tcp /srv/lustre/lost1

# Mount targets
sudo mkdir -p /lustre/mdt /lustre/ost0 /lustre/ost1

printf "\nMounting MDT...\n"
sudo mount -t lustre -o loop /srv/lustre/lmdt /lustre/mdt

printf "Mounting OST0...\n"
sudo mount -t lustre -o loop /srv/lustre/lost0 /lustre/ost0

printf "Mounting OST1...\n"
sudo mount -t lustre -o loop /srv/lustre/lost1 /lustre/ost1

# Mount client view of the filesystem
printf "\nMounting client FS...\n"
sudo mkdir -p /mnt/lustre
sudo mount.tlt -f /
sleep 2
sudo mount -t lustre ${MYIP}@tcp:/lfs /mnt/lustre
sudo chown ${IAM}.${IAM} /mnt/lustre/

# Sanity check
echo "hello lfsccm-rl9" > /mnt/lustre/sample
cat /mnt/lustre/sample
lfs df /mnt/lustre

printf "\n=== Lustre server is ready ===\n"
printf "Server IP : %s\n" "${MYIP}"
printf "Client MT : /mnt/lustre\n"
