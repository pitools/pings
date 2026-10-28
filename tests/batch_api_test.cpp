#include <cstdlib>
#include <exception>
#include <iostream>
#include <string_view>
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
}

int main()
{
    try
    {
        testEmptyBatch();
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