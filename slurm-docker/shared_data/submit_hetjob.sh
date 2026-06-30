#!/bin/bash
#SBATCH --job-name=hetjob_mpi
#SBATCH --nodes=4  
#SBATCH --ntasks-per-node=1 
#SBATCH --cpus-per-task=1
#SBATCH --time=01:00:00
#SBATCH --partition=mixed

#srun --mpi=pmix --exclusive \
#     --nodes=1 \
#     --ntasks=1 \
#     --mem-per-cpu=120 \
#     ./simple : --mpi=pmix --exclusive \
#     --nodes=3 \
#     --ntasks=3 \
#     --mem-per-cpu=20 \
#     ./simple

srun --mpi=pmix  \
     --nodes=1 \
     --ntasks=1 \
     --mem-per-cpu=120 \
     ./simple : --mpi=pmix  \
     --nodes=3 \
     --ntasks=3 \
     --mem-per-cpu=20 \
     ./simple


