#!/bin/bash
set -e

echo "---> Selecting BurstBuffer plugin: ${BB_PLUGIN:-pcc}"
if [ "$BB_PLUGIN" = "mpc" ]; then
    sed -i 's|BurstBufferType.*|BurstBufferType=burst_buffer/lua|' /etc/slurm/slurm.conf
    if ! grep -q "Directive=MPC" /etc/slurm/burst_buffer.conf 2>/dev/null; then
        echo 'Directive=MPC' >> /etc/slurm/burst_buffer.conf
    fi
else
    echo "---> Using legacy PCC Lua plugin"
fi

if [ "$1" = "slurmdbd" ]
then
    echo "---> Starting munged ..."
    gosu munge /usr/sbin/munged
    echo "---> Waiting for database ..."
    . /etc/slurm/slurmdbd.conf
    until echo "SELECT 1" | mysql -h $StorageHost -u$StorageUser -p$StoragePass 2>&1 > /dev/null
    do
        sleep 2
    done
    exec gosu slurm /usr/sbin/slurmdbd -Dvvv
fi

if [ "$1" = "slurmctld" ]
then
    echo "---> Starting munged ..."
    gosu munge /usr/sbin/munged
    until 2>/dev/null >/dev/tcp/slurmdbd/6819
    do
        sleep 2
    done
    export LD_PRELOAD="${LD_PRELOAD}:/usr/local/lib/libpreadv_hook.so"
    if /usr/sbin/slurmctld -V | grep -q '17.02'; then
        exec gosu slurm /usr/sbin/slurmctld -Dvvv
    else
        exec gosu slurm /usr/sbin/slurmctld -i -Dvvv
    fi
fi

if [ "$1" = "slurmd" ]
then
    echo "---> Starting munged ..."
    gosu munge /usr/sbin/munged
    until 2>/dev/null >/dev/tcp/slurmctld/6817
    do
        sleep 2
    done
    export LD_PRELOAD="${LD_PRELOAD}:/usr/local/lib/libpreadv_hook.so"
    exec /usr/sbin/slurmd -Dvvv
fi

exec "$@"
