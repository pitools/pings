#pragma once

#include <cstddef>
#include <string_view>

#include <pings/ping_statistics.hpp>

namespace pings
{
    enum class RepeatCompletionStatus
    {
        Completed,
        Cancelled,
        Stopped,
        Failed,
        InvalidOptions
    };

    struct RepeatSummary
    {
        std::size_t completed{};
        PingStatistics statistics{};
        RepeatCompletionStatus status{RepeatCompletionStatus::Completed};
    };

    constexpr std::string_view toString(RepeatCompletionStatus status) noexcept
    {
        switch (status)
        {
        case RepeatCompletionStatus::Completed:
            return "completed";

        case RepeatCompletionStatus::Cancelled:
            return "cancelled";

        case RepeatCompletionStatus::Stopped:
            return "stopped";

        case RepeatCompletionStatus::Failed:
            return "failed";

        case RepeatCompletionStatus::InvalidOptions:
            return "invalid_options";
        }

        return "unknown";
    }
} // namespace pings