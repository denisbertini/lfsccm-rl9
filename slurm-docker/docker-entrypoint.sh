#!/bin/bash
set -e

echo "---> Selecting BurstBuffer plugin: ${BB_PLUGIN:-lua}"
if [ "${BB_PLUGIN}" = "mpc" ]; then
    sed -i '/^BurstBufferType=/s|.*|BurstBufferType=burst_buffer/lustre_mpc|' /etc/slurm/slurm.conf
else
    echo "---> Using legacy PCC (LUA) plugin"
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
    echo "---> Waiting for slurmctld..."
    until 2>/dev/null >/dev/tcp/slurmctld/6817; do sleep 2; done

    if [ "${LUSTRE_ENABLED:-0}" = "1" ]; then
        echo "---> Waiting for Lustre mount ..."
        for i in $(seq 1 30); do
            df /ustre &>/dev/null && break
            sleep 2
        done
    fi

    export LD_PRELOAD="${LD_PRELOAD}:/usr/local/lib/libpreadv_hook.so"
    exec /usr/sbin/slurmd -Dvvv
fi

exec "$@"
