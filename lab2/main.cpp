#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using Matrix = std::vector<std::vector<double>>;

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
    Matrix& matrix,
    size_t size
) {
    std::ifstream input(file_name);

    if (!input) {
        std::cerr << "Ошибка: не удалось открыть файл " << file_name << '\n';
        return false;
    }

    matrix.assign(size, std::vector<double>(size));

    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            if (!(input >> matrix[i][j])) {
                std::cerr << "Ошибка чтения матрицы\n";
                return false;
            }
        }
    }

    return true;
}

bool SaveMatrix(
    const std::string& file_name,
    const Matrix& matrix
) {
    std::ofstream output(file_name);

    if (!output) {
        std::cerr << "Ошибка: не удалось создать файл " << file_name << '\n';
        return false;
    }

    output << std::setprecision(15);

    for (size_t i = 0; i < matrix.size(); ++i) {
        for (size_t j = 0; j < matrix[i].size(); ++j) {
            if (j > 0) {
                output << ' ';
            }

            output << matrix[i][j];
        }

        output << '\n';
    }

    return true;
}

void Multiply(
    const Matrix& first,
    const Matrix& second,
    Matrix& result
) {
    const size_t size = first.size();

    for (size_t i = 0; i < size; ++i) {
        for (size_t k = 0; k < size; ++k) {
            for (size_t j = 0; j < size; ++j) {
                result[i][j] += first[i][k] * second[k][j];
            }
        }
    }
}

int main() {
    const size_t size = 1000;

    const std::string first_matrix_file = "matrix_a.txt";
    const std::string second_matrix_file = "matrix_b.txt";
    const std::string result_matrix_file = "result.txt";

    GenerateMatrix(first_matrix_file, size);
    GenerateMatrix(second_matrix_file, size);

    Matrix matrix_a;
    Matrix matrix_b;

    if (!LoadMatrix(first_matrix_file, matrix_a, size)) {
        return 1;
    }

    if (!LoadMatrix(second_matrix_file, matrix_b, size)) {
        return 1;
    }

    Matrix result(
        size,
        std::vector<double>(size, 0.0)
    );

    const auto start = std::chrono::steady_clock::now();

    Multiply(matrix_a, matrix_b, result);

    const auto finish = std::chrono::steady_clock::now();

    if (!SaveMatrix(result_matrix_file, result)) {
        return 1;
    }

    const std::chrono::duration<double> spent = finish - start;

    const double time = spent.count();
    const double operations = 2.0 * size * size * size;
    const double gflops = operations / (time * 1e9);

    std::cout << "Размер матриц: "
              << size << " x " << size << '\n';

    std::cout << "Время умножения: "
              << std::fixed << std::setprecision(6)
              << time << " секунд\n";

    std::cout << "Количество операций: "
              << std::fixed << std::setprecision(0)
              << operations << " FLOP\n";

    std::cout << "Производительность: "
              << std::fixed << std::setprecision(4)
              << gflops << " GFLOPS\n";

    return 0;
}

