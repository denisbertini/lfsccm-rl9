#!/bin/bash
# Install Lustre server packages and prepare kmods for running kernel
set -eu

sudo dnf -y --nogpgcheck --enablerepo=lustre-server install \
  kmod-lustre \
  kmod-lustre-osd-ldiskfs \
  lustre \
  lustre-osd-ldiskfs-mount

# Recompile kmods against the currently-running kernel
echo "=== Running kmodtool prepare (may take a few minutes)... ==="
sudo /usr/sbin/kmodtool prepare

# Verify kernel modules are available
lsmod | grep lustre || echo "(modules not loaded yet — expected before setup)"

echo "=== install done. Reboot: limactl stop lustre-rl9 && limactl start lustre-rl9 ==="
