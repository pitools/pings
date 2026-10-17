#pragma once

#include <chrono>
#include <cstddef>

namespace pings
{

    struct PingStatistics
    {
        std::size_t attempts{};
        std::size_t sent{};
        std::size_t received{};
        std::size_t lost{};

         double lossPercent{};

        std::chrono::milliseconds min{};
        std::chrono::milliseconds max{};
        std::chrono::milliseconds avg{};
    };

} // namespace pings