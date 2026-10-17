#pragma once

#include <functional>

#include <pings/ping_result.hpp>

namespace pings
{
    using PingCallback =
        std::function<void(const PingResult &result)>;
}