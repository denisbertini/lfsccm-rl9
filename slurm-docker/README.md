# Slurm-docker
This repository implements a toy Slurm cluster that mimic the **ORION** cluster environment, adapted from the original [multi-container slurm-cluster](https://github.com/giovtorres/slurm-docker-cluster)

## Motivations
Debugging a complex HPC Slurm cluster like **ORION** is a challenging task.
When a feature fails, isolating the root cause wheter it's the OS, 
software packaging/version or even the containerization technology 
(apptainer layer) can be a slow, complex and frustrating process.

This project is meant for rapid feature testing, by providing a
lightweight, fully containerized Slurm cluster that can run seamlessly 
on a local  desktop or even a laptop.

It also provides unrestricted access to the full range of SLurm parameter, 
allowing for safe experimentations and optimizations without contackt 
any system a\administrators.

Furthermore Docker Compose's basic support for image versioning allows for 
easy feature testing across many different version of Slurm, PMIx, OpenMPI. 
The combination of Slurm-PMIX plugin, PMIx itself, and openMPI because of its 
inherent complexite, being often the most sensistive and failure-prone part of 
the cluster stack


## Building a Slurm Cluster
The version of the OS, SLurm and the whole software stack is defined in the `Dockerfile` from this repository.
To build the image:

```
docker compose build
```

When dealing with multiple versions, to decide which will be tested,  a `.env` file is used automatically by
Docker Compose.
For example to fix the slurm version to `25.05.02`:

.env:
```
# Slurm git repo tag. See https://github.com/SchedMD/slurm/tags
SLURM_TAG=slurm-25-05-2-1

# Image version used to tag the container at build time (Typically matches the
# Slurm tag in semantic version form)
IMAGE_TAG=25-05-2-1
```

- To start (containers in detached mode):

```
docker compose up -d 

```
- Check that all containers are started and running:

```
docker compose ps
```

If some container did not start or stopped after initialisation, debug using the logs

```
docker compose logs <service:container_name>
```

Logging of all service/compute nodes can also be checked in real-time:

```
docker compose logs -f 
```

- Add a new cluster named "orion" to the Slurm database (slurmdbd) and then restarts 
the necessary services to apply the change. 

```
./register_cluster.sh
```

You can access any services/compute node opening a shell inside the corresponding container

slurmctld:
```
docker exec -it slurmctld bash
```

c1:
```
docker exec -it c1 bash
```

## Sending jobs
Login to the Slurm controller, and write/submit standard slurm scripts from the shared `/data` volume.
