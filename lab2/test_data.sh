#!/bin/bash

PROGRAM="./main"
OUTPUT_FILE="results.csv"

echo "Размер матрицы (NxN);Количество потоков;Время умножения (с);Количество операций (FLOP);Производительность (GFLOPS)" > $OUTPUT_FILE

SIZES=(5 10 100 200 400 600 800 1000 1600 2000)
THREADS=(1 2 4 8 16)

for size in "${SIZES[@]}"
do
    for threads in "${THREADS[@]}"
    do
        for run in 1 2 3
        do
            $PROGRAM $size $threads >> $OUTPUT_FILE
            
            check_result=$(python3 check.py)
            
            if [ "$check_result" = "Проверка не пройдена." ]; then
                echo "Ошибка обнаружена при параметрах -> Размер: ${size}x${size}, Потоков: ${threads}"
                exit 1
            fi
        done
    done
done

echo "Тест завершен без ошибок"

