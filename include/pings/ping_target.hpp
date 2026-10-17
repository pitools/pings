#pragma once

#include <string>

#include <pings/ping_options.hpp>

namespace pings
{
    struct PingTarget
    {
        std::string host;
        PingOptions options;
    };
}