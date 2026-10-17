// app_config.hpp
#pragma once

#include <vector>

#include <pings/ping_options.hpp>
#include <pings/config/host_config.hpp>

namespace pings::config
{
    struct AppConfig
    {
        PingOptions defaults{};
        std::vector<HostConfig> hosts;
    };
}