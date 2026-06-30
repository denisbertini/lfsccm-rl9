#!/bin/bash
#SBATCH --ntasks=1 --cpus-per-task=1 --mem-per-cpu=120 --partition sta
#SBATCH hetjob
#SBATCH --ntasks=1 --cpus-per-task=1 --mem-per-cpu=50 --hint=multithread --partition sta

# the following must be consistent with above requirements
resources=' --ntasks=1 --cpus-per-task=1 --mem-per-cpu=120 --partition sta : --ntasks=1 --cpus-per-task=1 --mem-per-cpu=20 --hint=multithread --partition sta'

# launch job
srun --mpi=pmix $resources hostname
