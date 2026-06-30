#!/bin/bash
#SBATCH --ntasks=1 --cpus-per-task=1 --mem-per-cpu=120 --partition sta
#SBATCH hetjob
#SBATCH --ntasks=1 --cpus-per-task=1 --mem-per-cpu=20 --hint=multithread --partition sta

# launch job
srun --mpi=pmix $SLURM_SUBMIT_DIR/simple : --mpi=pmix $SLURM_SUBMIT_DIR/simple 

