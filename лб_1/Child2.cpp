#include <windows.h>
#include <iostream>

#define PIPE2_IN "\\\\.\\pipe\\Pipe2_IN"
#define PIPE2_OUT "\\\\.\\pipe\\Pipe2_OUT"

void ErrorExit(const char* msg) {
    std::cerr << msg << " (" << GetLastError() << ")" << std::endl;
    exit(1);
}

int main() {
    setlocale(LC_ALL, "RUS");

    if (!WaitNamedPipeA(PIPE2_IN, NMPWAIT_WAIT_FOREVER)) ErrorExit("Не удалось дождаться PIPE2_IN");

    HANDLE hPipe2_IN = CreateFileA(PIPE2_IN, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hPipe2_IN == INVALID_HANDLE_VALUE) ErrorExit("Не удалось открыть Pipe2_IN");

    HANDLE hPipe2_OUT = CreateFileA(PIPE2_OUT, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hPipe2_OUT == INVALID_HANDLE_VALUE) ErrorExit("Не удалось открыть Pipe2_OUT");

    char buffer[1024] = { 0 };
    DWORD read, written;

    while (ReadFile(hPipe2_IN, buffer, sizeof(buffer) - 1, &read, nullptr)) {
        for (DWORD i = 0; i < read; i++) {
            if (buffer[i] == ' ') buffer[i] = '_';
        }
        WriteFile(hPipe2_OUT, buffer, read, &written, nullptr);
    }

    CloseHandle(hPipe2_IN);
    CloseHandle(hPipe2_OUT);
    return 0;
}
