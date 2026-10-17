#include <chrono>
#include <stdexcept>
#include <vector>

#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/chrono.h>

#include <pings/async_pinger.hpp>
#include <single_example.hpp>

namespace examples
{
    void runSingleExample()
    {
        fmt::print("Async Ping with single ping example\n");

        auto callback =
            [&](const pings::PingResult &result)
        {
            const auto latencyText =
                result.latency
                    ? fmt::format("{}ms", result.latency->count())
                    : std::string{"—"};
            const auto address = result.address ? result.address->to_string() : "unresolved";

            fmt::print(
                "Result: ip: {}, {}, status={}\n",
                address,
                latencyText,
                pings::toString(result.status));
        };
        auto pinger = pings::AsyncPinger::create();

        [[maybe_unused]] auto firstPing = pinger->pingAsync(
            "ya.ru",
            callback);

        [[maybe_unused]] auto secondPing = pinger->pingAsync(
            "::1",
            callback);

        pinger->run();
    }
}
