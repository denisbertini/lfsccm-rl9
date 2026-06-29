#!/bin/bash
# Teardown Lustre filesystem (destroys all data)
set -eu

echo "Unmounting Lustre targets..."
sudo umount -f /mnt/lustre 2>/dev/null || true
sudo umount -f /lustre/ost0  2>/dev/null || true
sudo umount -f /lustre/ost1  2>/dev/null || true
sudo umount -f /lustre/mdt   2>/dev/null || true

echo "Removing loopback data..."
sudo rm -rf /srv/lustre

printf "\n=== Teardown complete ===\n"
