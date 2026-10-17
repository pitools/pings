// utils/console.hpp
#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

namespace utils {
    inline void setup_console() {
#ifdef _WIN32
        // Минимальный набор для работы
        SetConsoleOutputCP(CP_UTF8);

        // Включаем ANSI цвета
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
#else
        // Linux/macOS всё и так работает
#endif
    }
}