#include <windows.h>
#include <iostream>

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
    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);
    if (!hMapFile) ErrorExit("Не удалось открыть разделяемую память");

    char* sharedMemory = (char*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (!sharedMemory) ErrorExit("Не удалось отобразить разделяемую память");

    HANDLE semEmpty = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, SEM_EMPTY_NAME);
    HANDLE semFull = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, SEM_FULL_NAME);
    if (!semEmpty || !semFull) ErrorExit("Не удалось открыть семафоры");

    while (true) {
        WaitForSingleObject(semFull, INFINITE);

        for (int i = 0; sharedMemory[i] != '\0'; i++)
            if (sharedMemory[i] == ' ') sharedMemory[i] = '_';

        ReleaseSemaphore(semFull, 1, nullptr);
    }

    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapFile);
    return 0;
}
