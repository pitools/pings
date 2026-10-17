#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <string_view>
#include <thread>
#include <utility>

#include <fmt/core.h>
#include <fmt/color.h>

namespace utils
{
    enum class LogType
    {
        Info,
        Warning,
        Error
    };

    // Мьютекс для защиты вывода в консоль из разных потоков
    inline std::mutex &getLogMutex()
    {
        static std::mutex mtx;
        return mtx;
    }

    // Вспомогательная функция для получения короткого ID потока
    inline std::uint32_t getShortThreadId()
    {
        // Хешируем стандартный id потока, чтобы получить красивое короткое число вместо длинной строки
        auto id = std::this_thread::get_id();
        return static_cast<std::uint32_t>(std::hash<std::thread::id>{}(id) % 10000);
    }

    // Делаем функцию шаблонной, чтобы она сама идеально на лету упаковывала любые аргументы
    // Профессиональная функция: использует fmt::format_string для проверки {} на этапе компиляции
    template <typename... Args>

    inline void logWrite(
        LogType type,
        std::string_view file,
        int line,
        fmt::format_string<Args...> fmtString,
        Args &&...args) noexcept
    {
        try
        { // Гарантируем, что только один поток пишет в консоль в данную секунду
            std::lock_guard<std::mutex> lock(getLogMutex());

            fmt::terminal_color color = fmt::terminal_color::black;
            const char *prefix = "";
            switch (type)
            {
            case LogType::Info:
                color = fmt::terminal_color::cyan;
                prefix = "[INFO ]";
                break;
            case LogType::Warning:
                color = fmt::terminal_color::yellow;
                prefix = "[WARN ]";
                break;
            case LogType::Error:
                color = fmt::terminal_color::red;
                prefix = "[ERROR]";
                break;
            }
            // Выводим цветной тег уровня лога
            fmt::print(stderr, fg(color), "{} ", prefix);
            // Выводим метаинформацию: [Поток] [Файл:Строка]
            fmt::print(stderr, fg(color), "[T:{:04d}] [{}:{}] ", getShortThreadId(), file, line);
            // Выводим само сообщение
            fmt::print(stderr, fg(color), fmtString.get(), std::forward<Args>(args)...);
            fmt::print(stderr, "\n");
        }
        catch(...)
        {
            // Диагностика не должна прерывать основную операцию.
            // При ошибке вывода сообщение теряется.
        }
    }

    constexpr std::string_view strip_path(std::string_view path)
    {
        auto pos = path.find_last_of("/\\");
        return pos == std::string_view::npos ? path : path.substr(pos + 1);
    }
} // namespace utils

#ifdef DEBUG_ENABLED

#define PINGS_DEBUG_LOG(level, fmt_str, ...)                \
    do                                                      \
    {                                                       \
        try                                                 \
        {                                                   \
            ::utils::logWrite(                              \
                level,                                      \
                ::utils::strip_path(__FILE__),              \
                __LINE__,                                   \
                fmt_str __VA_OPT__(, ) __VA_ARGS__);        \
        }                                                   \
        catch (...)                                         \
        {                                                   \
            /* Ошибка подготовки аргументов диагностики. */ \
        }                                                   \
    } while (0)

#define DEBUG_INFO(fmt_str, ...) \
    PINGS_DEBUG_LOG(             \
        ::utils::LogType::Info,  \
        fmt_str __VA_OPT__(, ) __VA_ARGS__)

#define DEBUG_WARN(fmt_str, ...)   \
    PINGS_DEBUG_LOG(               \
        ::utils::LogType::Warning, \
        fmt_str __VA_OPT__(, ) __VA_ARGS__)

#define DEBUG_ERROR(fmt_str, ...) \
    PINGS_DEBUG_LOG(              \
        ::utils::LogType::Error,  \
        fmt_str __VA_OPT__(, ) __VA_ARGS__)

#else

// Если Release — макрос превращается в пустоту и ничего не делает
#define DEBUG_INFO(...) ((void)0)
#define DEBUG_WARN(...) ((void)0)
#define DEBUG_ERROR(...) ((void)0)
#endif
