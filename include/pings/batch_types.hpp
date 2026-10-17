#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include <pings/ping_result.hpp>

namespace pings
{
    using BatchProgressCallback =
        std::function<void(
            std::size_t completed,
            std::size_t total)>;

    using BatchCompletionCallback =
        std::function<void(
            const std::vector<PingResult> &results)>;

}