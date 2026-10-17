#pragma once

#include <chrono>
#include <cstddef>

#include <pings/ping_options.hpp>

namespace pings
{
    struct RepeatOptions
    {
        // 0 = repeat until cancelled/stopped
        std::size_t count{4};

        // Пауза после завершения одного ping
        // перед запуском следующего
        std::chrono::milliseconds interval{1000};

        PingOptions ping{};
    };

    inline constexpr auto MinRepeatInterval = std::chrono::milliseconds{0};

    inline constexpr auto MaxRepeatInterval = std::chrono::minutes{10};

    [[nodiscard]]
    bool isValid(const RepeatOptions &options) noexcept;
}