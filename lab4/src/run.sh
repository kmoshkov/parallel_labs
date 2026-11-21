#!/bin/bash

SIZES=(200 400 800 1200 1600 2000)
BLOCKS=("4 4" "8 8" "16 16" "32 32" "32 8" "8 32" "64 8" "8 64")
RUNS=3
OUTPUT_FILE="results.csv"

nvcc -O3 main.cu -o cuda_matrix || exit 1

echo "Размер матрицы (NxN);Блок X;Блок Y;Время умножения (с);Количество операций (FLOP);Производительность (GFLOPS)" > $OUTPUT_FILE

for size in "${SIZES[@]}"
do
    ./cuda_matrix $size --generate

    for block in "${BLOCKS[@]}"
    do
        for run in $(seq 1 $RUNS)
        do
            ./cuda_matrix $size $block >> $OUTPUT_FILE
        done
    done
done

rm -f matrix_a.txt matrix_b.txt

