#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string_view>

#include <pings/async_pinger.hpp>

namespace
{
    int failures = 0;

    void expect(bool condition, std::string_view name)
    {
        if (!condition)
        {
            ++failures;
            std::cerr << "FAIL: " << name << '\n';
        }
    }

    void testInvalidRepeatOptions()
    {
        using namespace std::chrono_literals;

        auto pinger = pings::AsyncPinger::create();

        pings::RepeatOptions options{};
        options.interval = -1ms;

        int resultCallbacks = 0;
        int completionCallbacks = 0;
        pings::RepeatSummary summary{};

        [[maybe_unused]] auto handle = pinger->pingRepeated(
            "127.0.0.1",
            options,
            [&](const pings::PingResult &, std::size_t)
            {
                ++resultCallbacks;
            },
            [&](const pings::RepeatSummary &value)
            {
                ++completionCallbacks;
                summary = value;
            });

        expect(resultCallbacks == 0, "invalid repeat has no inline results");
        expect(completionCallbacks == 0, "repeat completion is deferred");

        pinger->run();

        expect(resultCallbacks == 0, "invalid repeat starts no attempts");
        expect(completionCallbacks == 1, "invalid repeat completes once");
        expect(
            summary.status == pings::RepeatCompletionStatus::InvalidOptions,
            "repeat reports invalid options");
        expect(summary.completed == 0, "no completed attempts");
        expect(summary.statistics.attempts == 0, "no recorded attempts");
        expect(summary.statistics.sent == 0, "no requests sent");

        pinger->run();

        expect(resultCallbacks == 0, "repeated run produces no results");
        expect(completionCallbacks == 1, "repeat completion is not repeated");
    }

    void testStopBeforeFirstSend()
    {
        auto pinger = pings::AsyncPinger::create();

        pings::RepeatOptions options{};

        int resultCallbacks = 0;
        int completionCallbacks = 0;
        pings::RepeatSummary summary{};

        pinger->stop();

        [[maybe_unused]] auto handle = pinger->pingRepeated(
            "127.0.0.1",
            options,
            [&](const pings::PingResult &, std::size_t)
            {
                ++resultCallbacks;
            },
            [&](const pings::RepeatSummary &value)
            {
                ++completionCallbacks;
                summary = value;
            });

        expect(resultCallbacks == 0, "stopped repeat has no inline results");
        expect(completionCallbacks == 0, "repeat completion is deferred");

        pinger->run();

        expect(resultCallbacks == 0, "stopped repeat delivers no attempt results");
        expect(completionCallbacks == 1, "stopped repeat completes once");
        expect(
            summary.status == pings::RepeatCompletionStatus::Stopped,
            "repeat reports stopped");
        expect(summary.completed == 0, "no completed attempts");
        expect(summary.statistics.attempts == 0, "no recorded attempts");
        expect(summary.statistics.sent == 0, "no requests sent");

        pinger->run();

        expect(resultCallbacks == 0, "repeated run produces no results");
        expect(completionCallbacks == 1, "repeat completion is not repeated");
    }
}

int main()
{
    try
    {
        testInvalidRepeatOptions();
        testStopBeforeFirstSend();
    }
    catch (const std::exception &error)
    {
        ++failures;
        std::cerr << "Unexpected exception: " << error.what() << '\n';
    }
    catch (...)
    {
        ++failures;
        std::cerr << "Unexpected non-standard exception\n";
    }

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}