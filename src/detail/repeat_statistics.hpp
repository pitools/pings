#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>

#include <pings/ping_result.hpp>
#include <pings/ping_statistics.hpp>

namespace pings::detail
{
    inline void recordRepeatResult(
        PingStatistics &statistics,
        std::chrono::milliseconds &totalLatency,
        const PingResult &result)
    {
        ++statistics.attempts;

        if (result.requestSent)
        {
            ++statistics.sent;
        }

        if (result.status == PingStatus::Success && result.latency)
        {
            ++statistics.received;
            totalLatency += *result.latency;

            if (statistics.received == 1)
            {
                statistics.min = *result.latency;
                statistics.max = *result.latency;
            }
            else
            {
                statistics.min =
                    std::min(statistics.min, *result.latency);

                statistics.max =
                    std::max(statistics.max, *result.latency);
            }

            statistics.avg =
                totalLatency /
                static_cast<std::int64_t>(statistics.received);
        }

        // Для корректного PingResult:
        // Success означает requestSent == true и наличие latency.
        statistics.lost =
            statistics.sent - statistics.received;

        statistics.lossPercent =
            statistics.sent == 0
                ? 0.0
                : 100.0 *
                      static_cast<double>(statistics.lost) /
                      static_cast<double>(statistics.sent);
    }
}