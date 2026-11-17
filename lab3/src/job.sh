#!/bin/bash
#SBATCH --job-name=mpi_matrix_lab
#SBATCH --output=job_output.txt
#SBATCH --error=job_errors.err
#SBATCH --partition=batch
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=16
#SBATCH --time=00:40:00

module purge
module load intel/icc18
module load intel/mpi5

OUTPUT_FILE="results.csv"

SIZES=(200 400 800 1200 1600 2000)
CORES=(1 2 4 8 12 16)
RUNS=3

echo "Размер матрицы (NxN);Количество процессов;Время умножения (с);Количество операций (FLOP);Производительность (GFLOPS)" > $OUTPUT_FILE

echo "Узел: $(hostname)"
echo "Процессор: $(lscpu | grep 'Model name' | sed -E 's/.*:\s+//')"
echo "Старт: $(date)"

for size in "${SIZES[@]}"
do
    mpirun -np 1 ./mpi_matrix $size --generate

    for cores in "${CORES[@]}"
    do
        for run in $(seq 1 $RUNS)
        do
            mpirun -np $cores -bootstrap slurm ./mpi_matrix $size >> $OUTPUT_FILE
        done
        echo "Готово: N=$size, процессов=$cores"
    done
done

rm -f matrix_a.txt matrix_b.txt

echo "Конец: $(date)"
