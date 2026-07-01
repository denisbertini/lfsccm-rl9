#!/bin/bash
# Pre-install — lightweight setup only (repo config + small deps)
# Heavy package installs (Docker CE, kernel-devel) are kept separate
# so individual steps stay within tool timeout windows.
set -eux

# Disable SELinux — required for loopback devices used by Lustre MDT
sudo sed -i 's/^SELINUX=.*/SELINUX=disabled/' /etc/selinux/config

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

echo "=== repos configured ==="
