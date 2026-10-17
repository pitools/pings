#include <pings/config/config_mapping.hpp>

namespace pings::config
{
    PingOptions makePingOptions(
        const PingOptions &defaults,
        const PingOptionsOverride &overrides)
    {
        PingOptions result = defaults;

        if (overrides.replyTimeout)
        {
            result.replyTimeout = *overrides.replyTimeout;
        }

        if (overrides.resolveTimeout)
        {
            result.resolveTimeout = *overrides.resolveTimeout;
        }

        if (overrides.payloadSize)
        {
            result.payloadSize = *overrides.payloadSize;
        }

        if (overrides.addressFamily)
        {
            result.addressFamily = *overrides.addressFamily;
        }

        return result;
    }

    PingTarget makePingTarget(
        const HostConfig &host,
        const PingOptions &defaults)
    {
        return {
            host.host,
            makePingOptions(defaults, host.ping)};
    }

    std::vector<PingTarget> makePingTargets(
        const AppConfig &config)
    {
        std::vector<PingTarget> targets;

        targets.reserve(config.hosts.size());

        for (const auto &host : config.hosts)
        {
            if (!host.enabled)
            {
                continue;
            }

            targets.push_back(
                makePingTarget(
                    host,
                    config.defaults));
        }

        return targets;
    }
}