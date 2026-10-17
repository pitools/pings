#pragma once

#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace utils {
    inline std::string to_utf8(const std::string& native_string) {
#ifdef _WIN32
        // Windows: CP1251 / OEM → UTF-8
        if (native_string.empty()) return {};

        int wide_len = MultiByteToWideChar(CP_ACP, 0, native_string.c_str(), -1, nullptr, 0);
        if (wide_len <= 0) return native_string;

        std::wstring wide(wide_len, L'\0');
        MultiByteToWideChar(CP_ACP, 0, native_string.c_str(), -1, wide.data(), wide_len);

        int utf8_len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8_len <= 0) return native_string;

        std::string utf8(utf8_len, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), utf8_len, nullptr, nullptr);

        if (!utf8.empty() && utf8.back() == '\0') utf8.pop_back();
        return utf8;
#else
        // Linux/macOS: уже UTF-8
        return native_string;
#endif
    }

    // Для std::error_code
    inline std::string error_to_utf8(const std::error_code& ec) {
        return to_utf8(ec.message());
    }

    // Для boost::system::error_code
    inline std::string error_to_utf8(const boost::system::error_code& ec) {
        return to_utf8(ec.message());
    }
}