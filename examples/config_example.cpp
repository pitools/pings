#include <chrono>
#include <vector>

#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/chrono.h>

#include <pings/async_pinger.hpp>
#include <config_example.hpp>

#include <pings/config/app_config.hpp>
#include <pings/config/config_loader.hpp>
#include <pings/config/config_mapping.hpp>
#include <pings/config/config_validation.hpp>

namespace examples
{
    int runConfigExample()
    {
        fmt::print("Async pings example with config file\n");
        const auto loadResult =
            pings::config::loadConfig("config.yml");

        if (!loadResult.success())
        {
            for (const auto &error : loadResult.errors)
            {
                fmt::print(
                    "Config load error [{}] at {}:{}: {}\n",
                    pings::config::toString(error.code),
                    error.line,
                    error.column,
                    error.message);
            }

            return 1;
        }

        const auto &config =
            *loadResult.config;

        const auto validation =
            pings::config::validate(config);

        if (!validation.valid())
        {
            for (const auto &error : validation.errors)
            {
                fmt::print(
                    "Config validation error [{}]: {}\n",
                    error.field,
                    error.message);
            }

            return 1;
        }

        const auto targets =
            pings::config::makePingTargets(config);

        auto pinger = pings::AsyncPinger::create();

        auto batch =
            pinger->pingBatch(
                targets,
                [](std::size_t completed, std::size_t total)
                {
                    fmt::print(
                        "Progress: {}/{}\n",
                        completed,
                        total);
                },
                [](const std::vector<pings::PingResult> &results)
                {
                    fmt::print("\nBatch completed:\n");

                    for (const auto &result : results)
                    {
                        const auto address =
                            result.address
                                ? result.address->to_string()
                                : "unresolved";

                        const auto latencyText =
                            result.latency
                                ? fmt::format("{} ms", result.latency->count())
                                : std::string{"—-"};

                        fmt::print(
                            "{} -> {} -> {} -> {}\n",
                            result.host,
                            latencyText,
                            address,
                            pings::toString(result.status));
                    }
                });

        pinger->run();

        return 0;
    }
}
