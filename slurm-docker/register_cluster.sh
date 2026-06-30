#!/bin/bash
set -e

docker exec slurmctld bash -c "/usr/bin/sacctmgr --immediate add cluster name=orion" && \
docker-compose restart slurmdbd slurmctld
