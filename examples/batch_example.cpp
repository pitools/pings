#include <chrono>
#include <vector>

#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/chrono.h>

#include <pings/async_pinger.hpp>
#include <batch_example.hpp>

namespace examples
{
    void runBatchExample()
    {
        // pings::single
        fmt::print("Async Ping with Single Thread, русский\n");

        try
        {
            auto pinger = pings::AsyncPinger::create();

            pings::PingOptions options{};
            options.addressFamily = pings::AddressFamily::IPv6;

            options.replyTimeout = std::chrono::milliseconds{1000};
            options.resolveTimeout = std::chrono::milliseconds{3000};
            options.payloadSize = 32;

            // pingBatch
            std::vector<pings::PingTarget> targets = {
                // {"127.0.0.1", options},
                // {"::1", options},
                {"google.com", options}/*,
                /*{"2a00:1450:4010:c03::8a", options}*/};

            auto batch = pinger->pingBatch(
                targets,
                [](std::size_t completed, std::size_t total)
                {
                    fmt::print(
                        "Progress: {}/{}\n", completed, total);
                },
                [](const std::vector<pings::PingResult> &results)
                {
                    fmt::print(
                        "\nBatch completed:\n");

                    for (const auto &result : results)
                    {
                        const auto address = result.address ? result.address->to_string() : "unresolved";

                        const auto latencyText =
                            result.latency
                                ? fmt::format("{}ms", result.latency->count())
                                : std::string{"—"};

                        fmt::print(
                            "{} -> {} -> {} -> {}\n",
                            result.host,
                            address,
                            pings::toString(result.status),
                            latencyText);
                    }
                });

            pinger->run();
        }
        catch (const boost::system::system_error &e)
        {
            fmt::print("System error in boost: {} ({})\n", e.code().value(), e.what());
        }
        catch (const std::exception &e)
        {
            fmt::print("Exception: {}\n", e.what());
        }
        catch (...)
        {
            fmt::print("Unknown exception\n");
        }
    }
}
