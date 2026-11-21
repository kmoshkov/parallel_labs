#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <cuda_runtime.h>

bool GenerateMatrix(const std::string& filename, size_t size) {
    std::ofstream output(filename);

    if (!output.is_open()) {
        std::cerr << "Ошибка: не удалось создать файл " << filename << '\n';
        return false;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(-50.0, 50.0);

    output << std::setprecision(15);

    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            if (j > 0) {
                output << ' ';
            }

            output << dis(gen);
        }

        output << '\n';
    }
    return true;
}

bool LoadMatrix(
    const std::string& file_name,
    std::vector<double>& matrix,
    size_t size
) {
    std::ifstream input(file_name);

    if (!input) {
        std::cerr << "Ошибка: не удалось открыть файл " << file_name << '\n';
        return false;
    }

    matrix.assign(size * size, 0.0);

    for (size_t i = 0; i < size * size; ++i) {
        if (!(input >> matrix[i])) {
            std::cerr << "Ошибка чтения матрицы\n";
            return false;
        }
    }

    return true;
}

__global__ void Multiply(
    const double* first,
    const double* second,
    double* result,
    int size
) {
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int row = blockIdx.y * blockDim.y + threadIdx.y;

    if (row < size && col < size) {
        double sum = 0.0;

        for (int k = 0; k < size; ++k) {
            sum += first[row * size + k] * second[k * size + col];
        }

        result[row * size + col] = sum;
    }
}

bool CheckCuda(cudaError_t status) {
    if (status != cudaSuccess) {
        std::cerr << "Ошибка CUDA: " << cudaGetErrorString(status) << '\n';
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    size_t size = 1000;
    int block_x = 16;
    int block_y = 16;

    if (argc >= 2) {
        size = std::stoi(argv[1]);
    }

    const std::string first_matrix_file = "matrix_a.txt";
    const std::string second_matrix_file = "matrix_b.txt";

    if (argc >= 3 && std::string(argv[2]) == "--generate") {
        GenerateMatrix(first_matrix_file, size);
        GenerateMatrix(second_matrix_file, size);
        return 0;
    }

    if (argc >= 4) {
        block_x = std::stoi(argv[2]);
        block_y = std::stoi(argv[3]);
    }

    cudaDeviceProp prop;
    if (!CheckCuda(cudaGetDeviceProperties(&prop, 0))) {
        return 1;
    }

    if (block_x * block_y > prop.maxThreadsPerBlock) {
        std::cerr << "Ошибка: в блоке не может быть больше "
                  << prop.maxThreadsPerBlock << " потоков\n";
        return 1;
    }

    std::vector<double> matrix_a;
    std::vector<double> matrix_b;

    if (!LoadMatrix(first_matrix_file, matrix_a, size)) {
        return 1;
    }

    if (!LoadMatrix(second_matrix_file, matrix_b, size)) {
        return 1;
    }

    std::vector<double> result(size * size, 0.0);

    const size_t bytes = size * size * sizeof(double);

    double* device_a = nullptr;
    double* device_b = nullptr;
    double* device_result = nullptr;

    if (!CheckCuda(cudaFree(0))) {
        return 1;
    }

    if (!CheckCuda(cudaMalloc(&device_a, bytes)) ||
        !CheckCuda(cudaMalloc(&device_b, bytes)) ||
        !CheckCuda(cudaMalloc(&device_result, bytes))) {
        return 1;
    }

    cudaMemcpy(device_a, matrix_a.data(), bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(device_b, matrix_b.data(), bytes, cudaMemcpyHostToDevice);

    dim3 block(block_x, block_y);
    dim3 grid(
        (size + block_x - 1) / block_x,
        (size + block_y - 1) / block_y
    );

    cudaEvent_t start;
    cudaEvent_t finish;
    cudaEventCreate(&start);
    cudaEventCreate(&finish);

    cudaEventRecord(start);
    Multiply<<<grid, block>>>(
        device_a, device_b, device_result, static_cast<int>(size)
    );
    cudaEventRecord(finish);
    cudaEventSynchronize(finish);

    if (!CheckCuda(cudaGetLastError())) {
        return 1;
    }

    float milliseconds = 0.0f;
    cudaEventElapsedTime(&milliseconds, start, finish);

    cudaMemcpy(result.data(), device_result, bytes, cudaMemcpyDeviceToHost);

    const double time = milliseconds / 1000.0;
    const double operations = 2.0 * size * size * size;
    const double gflops = operations / (time * 1e9);

    std::cout << size << ";"
              << block_x << ";"
              << block_y << ";"
              << std::fixed << std::setprecision(6) << time << ";"
              << std::fixed << std::setprecision(0) << operations << ";"
              << std::fixed << std::setprecision(4) << gflops << '\n';

    cudaEventDestroy(start);
    cudaEventDestroy(finish);
    cudaFree(device_a);
    cudaFree(device_b);
    cudaFree(device_result);

    return 0;
}
