#include <windows.h>
#include <iostream>
#include <cctype>

#define PIPE1 "\\\\.\\pipe\\Pipe1"
#define PIPE2_IN "\\\\.\\pipe\\Pipe2_IN"

void ErrorExit(const char* msg) {
    std::cerr << msg << " (" << GetLastError() << ")" << std::endl;
    exit(1);
}

int main() {
    setlocale(LC_ALL, "RUS");
    if (!WaitNamedPipeA(PIPE1, NMPWAIT_WAIT_FOREVER)) ErrorExit("Не удалось дождаться PIPE1");

    HANDLE hPipe1 = CreateFileA(PIPE1, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hPipe1 == INVALID_HANDLE_VALUE) ErrorExit("Не удалось открыть Pipe1");

    HANDLE hPipe2_IN = CreateNamedPipeA(PIPE2_IN, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 0, 0, nullptr);
    if (hPipe2_IN == INVALID_HANDLE_VALUE) ErrorExit("Не удалось создать Pipe2_IN");

    char buffer[1024] = { 0 };
    DWORD read, written;

    while (ReadFile(hPipe1, buffer, sizeof(buffer) - 1, &read, nullptr)) {
        for (DWORD i = 0; i < read; i++) buffer[i] = std::tolower(buffer[i]);
        WriteFile(hPipe2_IN, buffer, read, &written, nullptr);
    }

    CloseHandle(hPipe1);
    CloseHandle(hPipe2_IN);
    return 0;
}
