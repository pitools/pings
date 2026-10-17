#pragma once

#include <cstddef>
#include <functional>

#include <pings/ping_result.hpp>
#include <pings/repeat_summary.hpp>

namespace pings
{
    using RepeatResultCallback = std::function<void(const PingResult &result, std::size_t index)>;

    using RepeatCompletionCallback = std::function<void(const RepeatSummary &summary)>;
}