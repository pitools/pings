#pragma once

#include <string_view>

namespace pings::detail
{
    enum class PingerState
    {
        Idle,       // Объект создан, run не запущен, новые операции разрешены
        Running,    // Выполняется обработка событий
        Stopping,   // Обрабатывается остановка, новые операции больше не принимаем, текущие операции завершаются
        Stopped,    // Работа завершена окончательно, io_context::run() завершён
        Interrupted // run() завершился исключением
    };

    constexpr std::string_view toString(PingerState state) noexcept
    {
        switch (state)
        {
        case PingerState::Idle:
            return "Idle";

        case PingerState::Running:
            return "Running";

        case PingerState::Stopping:
            return "Stopping";

        case PingerState::Stopped:
            return "Stopped";

        case PingerState::Interrupted:
            return "Interrupted";
        }

        return "Unknown";
    }
}
