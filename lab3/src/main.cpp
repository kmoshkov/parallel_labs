#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <mpi.h>

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

void Multiply(
    const std::vector<double>& first,
    const std::vector<double>& second,
    std::vector<double>& result,
    size_t rows,
    size_t size
) {
    for (size_t i = 0; i < rows; ++i) {
        for (size_t k = 0; k < size; ++k) {
            double a = first[i * size + k];
            for (size_t j = 0; j < size; ++j) {
                result[i * size + j] += a * second[k * size + j];
            }
        }
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank = 0;
    int num_procs = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    size_t size = 1000;
    bool need_generation = false;

    if (argc >= 2) {
        size = std::stoi(argv[1]);
    }

    if (argc >= 3 && std::string(argv[2]) == "--generate") {
        need_generation = true;
    }

    const std::string first_matrix_file = "matrix_a.txt";
    const std::string second_matrix_file = "matrix_b.txt";

    if (need_generation) {
        if (rank == 0) {
            GenerateMatrix(first_matrix_file, size);
            GenerateMatrix(second_matrix_file, size);
        }

        MPI_Finalize();
        return 0;
    }

    std::vector<int> send_counts(num_procs);
    std::vector<int> displacements(num_procs);

    const size_t base_rows = size / num_procs;
    const size_t extra_rows = size % num_procs;
    int offset = 0;

    for (int p = 0; p < num_procs; ++p) {
        size_t rows = base_rows + (static_cast<size_t>(p) < extra_rows ? 1 : 0);
        send_counts[p] = static_cast<int>(rows * size);
        displacements[p] = offset;
        offset += send_counts[p];
    }

    const size_t local_rows = send_counts[rank] / size;

    std::vector<double> matrix_a;
    std::vector<double> matrix_b(size * size);
    std::vector<double> result;

    std::vector<double> local_a(send_counts[rank]);
    std::vector<double> local_result(send_counts[rank], 0.0);

    if (rank == 0) {
        if (!LoadMatrix(first_matrix_file, matrix_a, size)) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        if (!LoadMatrix(second_matrix_file, matrix_b, size)) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        result.assign(size * size, 0.0);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    const auto start = std::chrono::steady_clock::now();

    MPI_Bcast(
        matrix_b.data(), static_cast<int>(size * size),
        MPI_DOUBLE, 0, MPI_COMM_WORLD
    );

    MPI_Scatterv(
        matrix_a.data(), send_counts.data(), displacements.data(), MPI_DOUBLE,
        local_a.data(), send_counts[rank], MPI_DOUBLE,
        0, MPI_COMM_WORLD
    );

    Multiply(local_a, matrix_b, local_result, local_rows, size);

    MPI_Gatherv(
        local_result.data(), send_counts[rank], MPI_DOUBLE,
        result.data(), send_counts.data(), displacements.data(), MPI_DOUBLE,
        0, MPI_COMM_WORLD
    );

    MPI_Barrier(MPI_COMM_WORLD);
    const auto finish = std::chrono::steady_clock::now();

    if (rank == 0) {
        const std::chrono::duration<double> spent = finish - start;

        const double time = spent.count();
        const double operations = 2.0 * size * size * size;
        const double gflops = operations / (time * 1e9);

        std::cout << size << ";"
                  << num_procs << ";"
                  << std::fixed << std::setprecision(6) << time << ";"
                  << std::fixed << std::setprecision(0) << operations << ";"
                  << std::fixed << std::setprecision(4) << gflops << '\n';
    }

    MPI_Finalize();
    return 0;
}
