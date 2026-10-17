// ping_options_override.hpp
#pragma once

#include <chrono>
#include <cstddef>
#include <optional>

#include <pings/ping_options.hpp>

namespace pings::config
{
    struct PingOptionsOverride
    {
        std::optional<std::chrono::milliseconds> replyTimeout;
        std::optional<std::chrono::milliseconds> resolveTimeout;
        std::optional<std::size_t> payloadSize;
        std::optional<AddressFamily> addressFamily;
    };
}