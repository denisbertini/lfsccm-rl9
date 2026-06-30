#!/bin/bash
# Pre-install: disable SELinux, add Lustre repos for EL9
set -eu

# Disable SELinux (required for loopback devices used by Lustre MDT)
sudo sed -i 's/^SELINUX=enforcing/SELINUX=disabled/g' /etc/selinux/config

# Add Whamcloud Lustre 2.17 + e2fsprogs-wc repos for Rocky/EL9
sudo sh -c "cat > /etc/yum.repos.d/lustre.repo <<\__EOF
[lustre-server]
name=lustre-server-el9
baseurl=https://downloads.whamcloud.com/public/lustre/lustre-2.17.0/el9.7/server
gpgcheck=0

[lustre-client]
name=lustre-client-el9
baseurl=https://downloads.whamcloud.com/public/lustre/lustre-2.17.0/el9.7/client
gpgcheck=0

[e2fsprogs-wc]
name=e2fsprogs-wc-el9
baseurl=https://downloads.whamcloud.com/public/e2fsprogs/latest/el9
gpgcheck=0
__EOF"

# Update system (exclude kernel packages to avoid conflicts)
sudo dnf update -y --exclude=kernel* --exclude=rocky-gpg-keys --exclude=centos-*

# Install base deps
sudo dnf -y install epel-release

# Install e2fsprogs from Whamcloud (required for ldiskfs backend)
sudo dnf -y --nogpgcheck --disablerepo=* --enablerepo=e2fsprogs-wc install e2fsprogs

# Install kernel-devel for kmodtool preparation later
sudo dnf -y --nogpgcheck install kernel-devel kernel-headers kernel-tools

echo "=== pre-install done. Now reboot: limactl stop lustre-rl9 && limactl start lustre-rl9 ==="
