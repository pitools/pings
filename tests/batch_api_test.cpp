#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

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

    void testEmptyBatch()
    {
        auto pinger = pings::AsyncPinger::create();

        int progressCallbacks = 0;
        int completionCallbacks = 0;
        bool resultsEmpty = false;

        [[maybe_unused]] auto handle = pinger->pingBatch(
            std::vector<pings::PingTarget>{},
            [&](std::size_t, std::size_t)
            {
                ++progressCallbacks;
            },
            [&](const std::vector<pings::PingResult> &results)
            {
                ++completionCallbacks;
                resultsEmpty = results.empty();
            });

        expect(completionCallbacks == 0, "empty batch completion is deferred");
        expect(progressCallbacks == 0, "empty batch has no inline progress");

        pinger->run();

        expect(completionCallbacks == 1, "empty batch completes once");
        expect(resultsEmpty, "empty batch returns no results");
        expect(progressCallbacks == 0, "empty batch has no progress");

        pinger->run();

        expect(completionCallbacks == 1, "empty batch completion is not repeated");
        expect(progressCallbacks == 0, "empty batch progress remains absent");
    }

    void testInvalidOptionsBatch()
    {
        using namespace std::chrono_literals;

        auto pinger = pings::AsyncPinger::create();

        pings::PingOptions options{};
        options.replyTimeout = -1ms;

        const std::vector<pings::PingTarget> targets{
            {"127.0.0.1", options},
            {"::1", options},
            {"127.0.0.2", options},
        };

        std::vector<std::pair<std::size_t, std::size_t>> progress;
        std::vector<pings::PingResult> results;
        int completionCallbacks = 0;
        std::size_t progressAtCompletion = 0;

        [[maybe_unused]] auto handle = pinger->pingBatch(
            targets,
            [&](std::size_t completed, std::size_t total)
            {
                progress.emplace_back(completed, total);
            },
            [&](const std::vector<pings::PingResult> &batchResults)
            {
                ++completionCallbacks;
                results = batchResults;
                progressAtCompletion = progress.size();
            });

        expect(progress.empty(), "batch progress is deferred");
        expect(completionCallbacks == 0, "batch completion is deferred");

        pinger->run();

        const std::vector<std::pair<std::size_t, std::size_t>>
            expectedProgress{{1, 3}, {2, 3}, {3, 3}};

        expect(progress == expectedProgress, "progress counts each target");
        expect(completionCallbacks == 1, "batch completes exactly once");
        expect(progressAtCompletion == 3, "final progress precedes completion");
        expect(results.size() == targets.size(), "one result per target");

        if (results.size() == targets.size())
        {
            for (std::size_t i = 0; i < targets.size(); ++i)
            {
                expect(
                    results[i].host == targets[i].host,
                    "result host matches input position");
                expect(
                    results[i].status == pings::PingStatus::InvalidOptions,
                    "each result reports invalid options");
                expect(!results[i].requestSent, "invalid target sends no request");
            }
        }

        pinger->run();

        expect(completionCallbacks == 1, "batch completion is not repeated");
        expect(progress == expectedProgress, "batch progress is not repeated");
    }

    void testProgressCallbackExceptionIsolation()
    {
        using namespace std::chrono_literals;

        auto pinger = pings::AsyncPinger::create();

        pings::PingOptions options{};
        options.replyTimeout = -1ms;

        const std::vector<pings::PingTarget> targets{
            {"127.0.0.1", options},
            {"::1", options},
            {"127.0.0.2", options},
        };

        std::vector<std::pair<std::size_t, std::size_t>> progress;
        std::vector<pings::PingResult> results;
        int completionCallbacks = 0;
        std::size_t progressAtCompletion = 0;

        [[maybe_unused]] auto handle = pinger->pingBatch(
            targets,
            [&](std::size_t completed, std::size_t total)
            {
                progress.emplace_back(completed, total);
                throw std::runtime_error{"Expected progress callback failure"};
            },
            [&](const std::vector<pings::PingResult> &batchResults)
            {
                ++completionCallbacks;
                results = batchResults;
                progressAtCompletion = progress.size();
            });

        expect(progress.empty(), "throwing progress is deferred");
        expect(completionCallbacks == 0, "completion is deferred");

        pinger->run();

        const std::vector<std::pair<std::size_t, std::size_t>>
            expectedProgress{{1, 3}, {2, 3}, {3, 3}};

        expect(progress == expectedProgress, "progress continues after exceptions");
        expect(completionCallbacks == 1, "completion survives progress exceptions");
        expect(progressAtCompletion == 3, "completion follows final throwing progress");
        expect(results.size() == targets.size(), "all batch results survive");

        if (results.size() == targets.size())
        {
            for (std::size_t i = 0; i < targets.size(); ++i)
            {
                expect(
                    results[i].host == targets[i].host,
                    "result position survives progress exception");
                expect(
                    results[i].status == pings::PingStatus::InvalidOptions,
                    "result status survives progress exception");
                expect(!results[i].requestSent, "no request was sent");
            }
        }

        pinger->run();

        expect(progress == expectedProgress, "throwing progress is not repeated");
        expect(completionCallbacks == 1, "completion is not repeated");
    }

    void testCompletionCallbackExceptionIsolation()
    {
        using namespace std::chrono_literals;

        auto pinger = pings::AsyncPinger::create();

        pings::PingOptions options{};
        options.replyTimeout = -1ms;

        int throwingCallbacks = 0;
        int followingCallbacks = 0;
        std::vector<pings::PingResult> firstResults;
        std::vector<pings::PingResult> secondResults;

        [[maybe_unused]] auto first = pinger->pingBatch(
            std::vector<pings::PingTarget>{{"127.0.0.1", options}},
            [&](const std::vector<pings::PingResult> &results)
            {
                ++throwingCallbacks;
                firstResults = results;
                throw std::runtime_error{"Expected batch completion failure"};
            });

        [[maybe_unused]] auto second = pinger->pingBatch(
            std::vector<pings::PingTarget>{{"127.0.0.2", options}},
            [&](const std::vector<pings::PingResult> &results)
            {
                ++followingCallbacks;
                secondResults = results;
            });

        expect(throwingCallbacks == 0, "throwing completion is deferred");
        expect(followingCallbacks == 0, "following completion is deferred");

        pinger->run();

        expect(throwingCallbacks == 1, "throwing completion runs once");
        expect(followingCallbacks == 1, "next batch survives completion exception");
        expect(firstResults.size() == 1, "throwing completion receives result");
        expect(secondResults.size() == 1, "following completion receives result");

        if (firstResults.size() == 1)
        {
            expect(firstResults[0].host == "127.0.0.1", "first batch host");
            expect(
                firstResults[0].status == pings::PingStatus::InvalidOptions,
                "first batch status");
            expect(!firstResults[0].requestSent, "first batch sends no request");
        }

        if (secondResults.size() == 1)
        {
            expect(secondResults[0].host == "127.0.0.2", "second batch host");
            expect(
                secondResults[0].status == pings::PingStatus::InvalidOptions,
                "second batch status");
            expect(!secondResults[0].requestSent, "second batch sends no request");
        }

        pinger->run();

        expect(throwingCallbacks == 1, "throwing completion is not retried");
        expect(followingCallbacks == 1, "following completion is not repeated");
    }

    void testStopPendingBatch()
    {
        auto pinger = pings::AsyncPinger::create();

        pings::PingOptions options{};

        const std::vector<pings::PingTarget> targets{
            {"127.0.0.1", options},
            {"::1", options},
            {"127.0.0.2", options},
        };

        std::vector<std::pair<std::size_t, std::size_t>> progress;
        std::vector<pings::PingResult> results;
        int completionCallbacks = 0;
        std::size_t progressAtCompletion = 0;

        pinger->stop();

        [[maybe_unused]] auto handle = pinger->pingBatch(
            targets,
            [&](std::size_t completed, std::size_t total)
            {
                progress.emplace_back(completed, total);
            },
            [&](const std::vector<pings::PingResult> &batchResults)
            {
                ++completionCallbacks;
                results = batchResults;
                progressAtCompletion = progress.size();
            });

        expect(progress.empty(), "batch progress is deferred");
        expect(completionCallbacks == 0, "batch completion is deferred");

        pinger->run();

        const std::vector<std::pair<std::size_t, std::size_t>>
            expectedProgress{{1, 3}, {2, 3}, {3, 3}};

        expect(progress == expectedProgress, "progress counts each target");
        expect(completionCallbacks == 1, "batch completes exactly once");
        expect(progressAtCompletion == 3, "final progress precedes completion");
        expect(results.size() == targets.size(), "one result per target");

        if (results.size() == targets.size())
        {
            for (std::size_t i = 0; i < targets.size(); ++i)
            {
                expect(
                    results[i].host == targets[i].host,
                    "result host matches input position");
                expect(
                    results[i].status == pings::PingStatus::Stopped,
                    "each result reports stopped");
                expect(!results[i].requestSent, "stopped target sends no request");
            }
        }

        pinger->run();

        expect(completionCallbacks == 1, "batch completion is not repeated");
        expect(progress == expectedProgress, "batch progress is not repeated");
    }
}

int main()
{
    try
    {
        testEmptyBatch();
        testInvalidOptionsBatch();
        testProgressCallbackExceptionIsolation();
        testCompletionCallbackExceptionIsolation();
        testStopPendingBatch();
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