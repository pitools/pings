#pragma once

#include <vector>

#include <pings/config/app_config.hpp>
#include <pings/config/host_config.hpp>
#include <pings/config/ping_options_override.hpp>
#include <pings/ping_options.hpp>
#include <pings/ping_target.hpp>

namespace pings::config
{
    [[nodiscard]]
    PingOptions makePingOptions(
        const PingOptions &defaults,
        const PingOptionsOverride &overrides);

    [[nodiscard]]
    PingTarget makePingTarget(
        const HostConfig &host,
        const PingOptions &defaults);

    [[nodiscard]]
    std::vector<PingTarget> makePingTargets(
        const AppConfig &config);
}