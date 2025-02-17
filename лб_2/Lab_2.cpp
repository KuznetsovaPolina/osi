#include <iostream>
#include <vector>
#include <windows.h>
#include <string>
#include <time.h>

struct ThreadData {
    const std::vector<std::vector<double>>* input;
    const std::vector<std::vector<double>>* filter;
    std::vector<std::vector<double>>* output;
    int row;
    int col;
};

DWORD WINAPI ApplyFilter(LPVOID param) {
    ThreadData* data = (ThreadData*)param;
    int filterSize = data->filter->size();
    double sum = 0.0;

    for (int i = 0; i < filterSize; i++) {
        for (int j = 0; j < filterSize; j++) {
            sum += (*data->input)[data->row + i][data->col + j] * (*data->filter)[i][j];
        }
    }
    (*data->output)[data->row][data->col] = sum;
    return 0;
}

void ApplyConvolution(std::vector<std::vector<double>>& matrix,
    const std::vector<std::vector<double>>& filter,
    int maxThreads, int K) {
    int filterSize = filter.size();

    for (int k = 0; k < K; k++) { 
        int outputSize = matrix.size() - filterSize + 1;
        if (outputSize <= 0) {
            std::cerr << "Матрица слишком мала для свёртки. Ранняя остановка на итерации " << k << "\n";
            break;
        }

        std::vector<std::vector<double>> result(outputSize, std::vector<double>(outputSize, 0.0));
        std::vector<HANDLE> threads;
        std::vector<ThreadData> threadData(outputSize * outputSize);
        int index = 0;

        for (int row = 0; row < outputSize; row++) {
            for (int col = 0; col < outputSize; col++) {
                threadData[index] = { &matrix, &filter, &result, row, col };
                HANDLE hThread = CreateThread(NULL, 0, ApplyFilter, &threadData[index], 0, NULL);
                threads.push_back(hThread);

                if (threads.size() >= maxThreads) {
                    WaitForMultipleObjects(threads.size(), threads.data(), TRUE, INFINITE);
                    for (HANDLE h : threads) CloseHandle(h);
                    threads.clear();
                }
                index++;
            }
        }

        if (!threads.empty()) {
            WaitForMultipleObjects(threads.size(), threads.data(), TRUE, INFINITE);
            for (HANDLE h : threads) CloseHandle(h);
        }

        matrix = result;
    }
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "RUS");
    clock_t tStart = clock();
    if (argc < 5) {
        std::cerr << "Использование: " << argv[0] << " <размер_матрицы> <размер_фильтра> <число_потоков> <повторения_К>\n";
        return 1;
    }

    int matrixSize = std::stoi(argv[1]);
    int filterSize = std::stoi(argv[2]); 
    int maxThreads = std::stoi(argv[3]);
    int K = std::stoi(argv[4]);

    if (matrixSize < filterSize) { 
        std::cerr << "Размер матрицы должен быть больше или равен размеру фильтра.\n";
        return 1;
    }

    std::vector<std::vector<double>> matrix(matrixSize, std::vector<double>(matrixSize, 1.0));
    std::vector<std::vector<double>> filter(filterSize, std::vector<double>(filterSize, 1.0));
    ApplyConvolution(matrix, filter, maxThreads, K);
    std::cout << "Время выполнения: " << ((double)(clock() - tStart) / CLOCKS_PER_SEC) << " сек" << std::endl;
    std::cout << "Обработка завершена.\n";
    return 0;
}
