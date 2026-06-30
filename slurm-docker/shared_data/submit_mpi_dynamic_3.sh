#!/bin/bash

#SBATCH --job-name=hetjob_mpi_dyn
#SBATCH --nodes=4  
#SBATCH --ntasks-per-node=1 
#SBATCH --cpus-per-task=1
#SBATCH --time=01:00:00
#SBATCH --partition=mixed


# launch job
srun --mpi=pmix --nodes=1 --ntasks=1 --cpus-per-task=1 --mem-per-cpu=120 --partition mixed  $SLURM_SUBMIT_DIR/simple : \
     --mpi=pmix --nodes=3 --ntasks=3 --cpus-per-task=1 --mem-per-cpu=20 --hint=multithread --partition mixed $SLURM_SUBMIT_DIR/simple


