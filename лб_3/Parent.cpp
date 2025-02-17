#include <windows.h>
#include <iostream>
#include <string>

#define SHARED_MEMORY_NAME "Global\\SharedMemory"
#define SEM_EMPTY_NAME "Global\\SemaphoreEmpty"
#define SEM_FULL_NAME "Global\\SemaphoreFull"
#define BUFFER_SIZE 1024

void ErrorExit(const char* msg) {
    std::cerr << msg << " (" << GetLastError() << ")" << std::endl;
    exit(1);
}

int main() {
    setlocale(LC_ALL, "RUS");
    HANDLE hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, BUFFER_SIZE, SHARED_MEMORY_NAME);
    if (!hMapFile) ErrorExit("Не удалось создать разделяемую память");

    char* sharedMemory = (char*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (!sharedMemory) ErrorExit("Не удалось отобразить разделяемую память");

    HANDLE semEmpty = CreateSemaphoreA(nullptr, 1, 1, SEM_EMPTY_NAME);
    HANDLE semFull = CreateSemaphoreA(nullptr, 0, 1, SEM_FULL_NAME);
    if (!semEmpty || !semFull) ErrorExit("Не удалось создать семафоры");

    STARTUPINFOA si1 = { sizeof(si1) }, si2 = { sizeof(si2) };
    PROCESS_INFORMATION pi1, pi2;

    if (!CreateProcessA("child1.exe", nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si1, &pi1))
        ErrorExit("Не удалось запустить child1");

    if (!CreateProcessA("child2.exe", nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si2, &pi2))
        ErrorExit("Не удалось запустить child2");

    std::string input;
    while (true) {
        std::cout << "Введите строку (или 'exit' для выхода): ";
        std::getline(std::cin, input);
        if (input == "exit") break;

        WaitForSingleObject(semEmpty, INFINITE);

        memset(sharedMemory, 0, BUFFER_SIZE);
        strcpy_s(sharedMemory, BUFFER_SIZE, input.c_str());

        ReleaseSemaphore(semFull, 1, nullptr);

        WaitForSingleObject(semFull, INFINITE);

        std::cout << "Результат: " << sharedMemory << std::endl;

        ReleaseSemaphore(semEmpty, 1, nullptr);
    }

    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapFile);
    CloseHandle(semEmpty);
    CloseHandle(semFull);
    CloseHandle(pi1.hProcess);
    CloseHandle(pi2.hProcess);

    return 0;
}
