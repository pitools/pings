// host_config.hpp
#pragma once

#include <string>

#include <pings/config/ping_options_override.hpp>

namespace pings::config
{
    struct HostConfig
    {
        std::string name;
        std::string host;

        bool enabled{true};

        PingOptionsOverride ping;
    };
}