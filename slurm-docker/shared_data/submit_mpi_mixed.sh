#!/bin/bash
#SBATCH --ntasks=1 --cpus-per-task=1 --mem-per-cpu=120 --partition sta
#SBATCH hetjob
#SBATCH --ntasks=1 --cpus-per-task=1 --mem-per-cpu=20 --hint=multithread --partition dyn

# the following must be consistent with above requirements
resources=' --ntasks=1 --cpus-per-task=1 --mem-per-cpu=120 --partition sta : --ntasks=1 --cpus-per-task=1 --mem-per-cpu=20 --hint=multithread --partition dyn'

# launch job
srun --mpi=pmix $resources $SLURM_SUBMIT_DIR/simple
