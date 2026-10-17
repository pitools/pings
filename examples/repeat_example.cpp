#include <chrono>
#include <vector>

#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/chrono.h>

#include <pings/async_pinger.hpp>
#include <repeat_example.hpp>

namespace examples
{
    void runRepeatExample()
    {
        // pings::single
        fmt::print("Async Ping with Single Thread, русский\n");

        try
        {
            auto pinger = pings::AsyncPinger::create();

            pings::PingOptions options;

            // Repeat test
            pings::RepeatOptions repeatOptions;
            repeatOptions.count = 3;
            repeatOptions.interval = std::chrono::milliseconds{250};
            repeatOptions.ping.replyTimeout = std::chrono::milliseconds(100);

            auto repeat =
                pinger->pingRepeated(
                    "127.0.0.1",
                    repeatOptions,
                    [](const pings::PingResult &result, std::size_t index)
                    {
                        fmt::print(
                            "Repeat #{}: {} -> {}\n",
                            index,
                            result.host,
                            toString(result.status));

                        // if (index == 1)
                        // {
                        //     throw std::runtime_error{"Test callback failure"};
                        // }
                    },
                    [](const pings::RepeatSummary &summary)
                    {
                        fmt::print(
                            "Attempts: {}, "
                            "completed: {}, "
                            "sent: {}, "
                            "received: {}, "
                            "lost: {}, "
                            "loss: {}%, "
                            "status: {}\n",
                            summary.statistics.attempts,
                            summary.completed,
                            summary.statistics.sent,
                            summary.statistics.received,
                            summary.statistics.lost,
                            summary.statistics.lossPercent,
                            toString(summary.status));

                        if (summary.statistics.received == 0)
                        {
                            fmt::print("RTT: нет измерений\n");
                        }
                        else
                        {
                            fmt::print(
                                "RTT: min={} ms, max={} ms, avg={} ms\n",
                                summary.statistics.min.count(),
                                summary.statistics.max.count(),
                                summary.statistics.avg.count());
                        }
                    }); // Repeat test

            fmt::print(">>> before run\n");

            pinger->run();

            fmt::print(">>> after run\n");
            // cancelThread.join();
            // cancelThread.join();
            pinger->run();

            pinger->stop();
            fmt::print("Program finished\n");
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
