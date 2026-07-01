#!/bin/bash
# Install Lustre server packages and prepare kmods for running kernel
set -eu

sudo dnf -y --nogpgcheck --enablerepo=lustre-server install \
  kmod-lustre \
  kmod-lustre-osd-ldiskfs \
  lustre \
  lustre-osd-ldiskfs-mount

# Lustre 2.17 ships precompiled kmods for its own kernel — boot into it
LUSTRE_KVER="5.14.0-611.13.1_lustre.el9.x86_64"
echo "=== Preparing Lustre kernel initramfs..."
sudo dracut --force --kver "$LUSTRE_KVER"
sudo grubby --set-default "/boot/vmlinuz-$LUSTRE_KVER"
echo "=== GRUB default set to Lustre kernel, reboot required ==="
ls "/lib/modules/$LUSTRE_KVER/extra/lustre/fs/*.ko" > /dev/null && \
    echo "Lustre kmods ready (will load after reboot into Lustre kernel)"

echo "=== lustre packages installed ==="
