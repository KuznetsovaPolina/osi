#include <windows.h>
#include <iostream>
#include <string>

#define PIPE1 "\\\\.\\pipe\\Pipe1"
#define PIPE2_OUT "\\\\.\\pipe\\Pipe2_OUT"

void ErrorExit(const char* msg) {
    std::cerr << msg << " (" << GetLastError() << ")" << std::endl;
    exit(1);
}

int main() {
    setlocale(LC_ALL, "RUS");

    HANDLE hPipe1 = CreateNamedPipeA(PIPE1, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 0, 0, nullptr);
    if (hPipe1 == INVALID_HANDLE_VALUE) ErrorExit("Не удалось создать Pipe1");

    HANDLE hPipe2_OUT = CreateNamedPipeA(PIPE2_OUT, PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 0, 0, nullptr);
    if (hPipe2_OUT == INVALID_HANDLE_VALUE) ErrorExit("Не удалось создать Pipe2_OUT");

    STARTUPINFOA si1 = { sizeof(si1) }, si2 = { sizeof(si2) };
    PROCESS_INFORMATION pi1, pi2;

    if (!CreateProcessA("Child1.exe", nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si1, &pi1))
        ErrorExit("Не удалось запустить child1");

    if (!CreateProcessA("Child2.exe", nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si2, &pi2))
        ErrorExit("Не удалось запустить child2");

    std::string input;
    while (true) {
        std::cout << "Введите строку (или 'exit' для выхода): ";
        std::getline(std::cin, input);
        if (input == "exit") break;

        DWORD written;
        WriteFile(hPipe1, input.c_str(), input.size(), &written, nullptr);

        char buffer[1024] = { 0 };
        DWORD read;
        ReadFile(hPipe2_OUT, buffer, sizeof(buffer) - 1, &read, nullptr);

        std::cout << "Результат: " << buffer << std::endl;
    }

    CloseHandle(hPipe1);
    CloseHandle(hPipe2_OUT);

    TerminateProcess(pi1.hProcess, 0);
    TerminateProcess(pi2.hProcess, 0);
    CloseHandle(pi1.hProcess);
    CloseHandle(pi2.hProcess);

    return 0;
}
