#!/bin/bash

#SBATCH --nodes=1 --ntasks=1 --cpus-per-task=1 --mem-per-cpu=120 --partition dyn
#SBATCH hetjob
#SBATCH --nodes=1 --ntasks=1 --cpus-per-task=1 --mem-per-cpu=20 --hint=multithread --partition dyn

# launch job
srun --mpi=pmix --nodes=1 --ntasks=1 --cpus-per-task=1 --mem-per-cpu=120 --partition dyn $SLURM_SUBMIT_DIR/simple : \
     --mpi=pmix --nodes=1 --ntasks=1 --cpus-per-task=1 --mem-per-cpu=20 --hint=multithread --partition dyn $SLURM_SUBMIT_DIR/simple



