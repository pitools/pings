#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
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

    void testDeferredCallbacksAndRestart()
    {
        using namespace std::chrono_literals;

        auto pinger = pings::AsyncPinger::create();

        pings::PingOptions options{};
        options.replyTimeout = -1ms;

        int callbackCount = 0;
        auto lastStatus = pings::PingStatus::InternalError;
        bool requestSent = true;

        const auto callback = [&](const pings::PingResult &result)
        {
            ++callbackCount;
            lastStatus = result.status;
            requestSent = result.requestSent;
        };

        [[maybe_unused]] auto first = pinger->pingAsync(
            "127.0.0.1", options, callback);

        expect(callbackCount == 0, "callback is deferred");

        pinger->run();

        expect(callbackCount == 1, "first callback occurs exactly once");
        expect(
            lastStatus == pings::PingStatus::InvalidOptions,
            "invalid options status");
        expect(!requestSent, "invalid operation sends no request");

        pinger->run();

        expect(callbackCount == 1, "empty run does not repeat callback");

        [[maybe_unused]] auto second = pinger->pingAsync(
            "127.0.0.1", options, callback);

        expect(callbackCount == 1, "second callback is deferred");

        pinger->run();

        expect(callbackCount == 2, "new work completes after restart");

        expect(lastStatus == pings::PingStatus::InvalidOptions,
               "second operation reports invalid options");
        expect(!requestSent, "second operation sends no request");

        pinger->run();

        expect(callbackCount == 2, "completed callbacks are not repeated");
    }

    void testStopAndRejection()
    {
        auto pinger = pings::AsyncPinger::create();

        int callbackCount = 0;
        auto lastStatus = pings::PingStatus::InternalError;
        bool requestSent = true;

        const auto callback = [&](const pings::PingResult &result)
        {
            ++callbackCount;
            lastStatus = result.status;
            requestSent = result.requestSent;
        };

        // Stopped run produces no extra callbacks
        pinger->stop();

        [[maybe_unused]] auto pending = pinger->pingAsync(
            "127.0.0.1", pings::PingOptions{}, callback);

        expect(callbackCount == 0, "stop completion is deferred");

        pinger->run();

        expect(callbackCount == 1, "stop completes pending operation once");
        expect(
            lastStatus == pings::PingStatus::Stopped,
            "pending operation reports stopped");
        expect(!requestSent, "stopped operation sends no request");

        bool rejected = false;

        try
        {
            [[maybe_unused]] auto afterStop = pinger->pingAsync(
                "127.0.0.1", pings::PingOptions{}, callback);
        }
        catch (const std::logic_error &)
        {
            rejected = true;
        }

        expect(rejected, "new work is rejected after stop");

        pinger->run();

        expect(callbackCount == 1, "stopped run produces no extra callbacks");
    }

    void testLateAndEmptyHandleCancellation()
    {
        // Late repeated cancellation produces no extra callbacks
        auto cancellationPinger = pings::AsyncPinger::create();
        int cancellationCallbacks = 0;
        auto cancellationStatus = pings::PingStatus::InternalError;

        pings::PingOptions mismatchOptions{};
        mismatchOptions.addressFamily = pings::AddressFamily::IPv6;

        auto completedHandle = cancellationPinger->pingAsync(
            "127.0.0.1",
            mismatchOptions,
            [&](const pings::PingResult &result)
            {
                ++cancellationCallbacks;
                cancellationStatus = result.status;
            });

        cancellationPinger->run();

        expect(cancellationCallbacks == 1, "family mismatch completes once");
        expect(
            cancellationStatus == pings::PingStatus::InvalidOptions,
            "family mismatch reports invalid options");

        completedHandle.cancel();
        completedHandle.cancel();

        cancellationPinger->run();

        expect(
            cancellationCallbacks == 1,
            "late repeated cancellation produces no extra callbacks");

        pings::PingHandle emptyHandle;
        emptyHandle.cancel();

        cancellationPinger->run();

        expect(
            cancellationCallbacks == 1,
            "empty handle cancellation produces no callbacks");
    }

    void testCallbackExceptionIsolation()
    {
        using namespace std::chrono_literals;

        auto pinger = pings::AsyncPinger::create();

        pings::PingOptions options{};
        options.replyTimeout = -1ms;

        int throwingCallbacks = 0;
        int followingCallbacks = 0;
        auto followingStatus = pings::PingStatus::InternalError;

        [[maybe_unused]] auto first = pinger->pingAsync(
            "127.0.0.1",
            options,
            [&](const pings::PingResult &)
            {
                ++throwingCallbacks;
                throw std::runtime_error{"Expected callback failure"};
            });

        [[maybe_unused]] auto second = pinger->pingAsync(
            "127.0.0.1",
            options,
            [&](const pings::PingResult &result)
            {
                ++followingCallbacks;
                followingStatus = result.status;
            });

        pinger->run();

        expect(throwingCallbacks == 1, "throwing callback runs once");
        expect(followingCallbacks == 1, "next callback survives exception");
        expect(
            followingStatus == pings::PingStatus::InvalidOptions,
            "next callback receives its result");

        pinger->run();

        expect(throwingCallbacks == 1, "throwing callback is not retried");
        expect(followingCallbacks == 1, "following callback is not repeated");
    }

    void testSessionCallbackExceptionIsolation()
    {
        auto pinger = pings::AsyncPinger::create();

        pings::PingOptions options{};
        options.addressFamily = pings::AddressFamily::IPv6;

        int throwingCallbacks = 0;
        int followingCallbacks = 0;
        auto throwingStatus = pings::PingStatus::InternalError;
        auto followingStatus = pings::PingStatus::InternalError;

        [[maybe_unused]] auto first = pinger->pingAsync(
            "127.0.0.1",
            options,
            [&](const pings::PingResult &result)
            {
                ++throwingCallbacks;
                throwingStatus = result.status;
                throw std::runtime_error{"Expected session callback failure"};
            });

        [[maybe_unused]] auto second = pinger->pingAsync(
            "127.0.0.1",
            options,
            [&](const pings::PingResult &result)
            {
                ++followingCallbacks;
                followingStatus = result.status;
            });

        expect(throwingCallbacks == 0, "session callback is deferred");
        expect(followingCallbacks == 0, "following session callback is deferred");

        pinger->run();

        expect(throwingCallbacks == 1, "throwing session callback runs once");
        expect(
            throwingStatus == pings::PingStatus::InvalidOptions,
            "throwing session reports family mismatch");
        expect(
            followingCallbacks == 1,
            "next session completes despite callback exception");
        expect(
            followingStatus == pings::PingStatus::InvalidOptions,
            "following session reports family mismatch");

        pinger->run();

        expect(
            throwingCallbacks == 1,
            "throwing session callback is not retried");
        expect(
            followingCallbacks == 1,
            "following session callback is not repeated");
    }

    void runScenario(std::string_view name, void (*scenario)())
    {
        const int failuresBefore = failures;

        try
        {
            scenario();
        }
        catch (const std::exception &error)
        {
            ++failures;
            std::cerr << "FAIL: " << name
                      << ": unexpected exception: "
                      << error.what() << '\n';
        }
        catch (...)
        {
            ++failures;
            std::cerr << "FAIL: " << name
                      << ": unexpected non-standard exception\n";
        }

        std::cout
            << (failures == failuresBefore ? "PASS: " : "FAIL: ")
            << name << '\n';
    }
}

int main()
{
    runScenario(
        "deferred callbacks and restart",
        testDeferredCallbacksAndRestart);

    runScenario(
        "stop and rejection",
        testStopAndRejection);

    runScenario(
        "late and empty handle cancellation",
        testLateAndEmptyHandleCancellation);

    runScenario(
        "callback exception isolation",
        testCallbackExceptionIsolation);

    runScenario(
        "session callback exception isolation",
        testSessionCallbackExceptionIsolation);

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
