#include <cstdlib>
#include <exception>
#include <iostream>
#include <string_view>
#include <utility>
#include <vector>

#include <batch_state.hpp>

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

    void testReverseCompletionOrder()
    {
        std::vector<pings::PingResult> input(3);
        input[0].host = "first";
        input[1].host = "second";
        input[2].host = "third";

        for (auto &result : input)
        {
            result.status = pings::PingStatus::InvalidOptions;
        }

        std::vector<pings::PingResult> results;
        std::vector<std::pair<std::size_t, std::size_t>> progress;
        int completionCallbacks = 0;

        pings::detail::BatchState state(
            input.size(),
            [&](const std::vector<pings::PingResult> &batchResults)
            {
                ++completionCallbacks;
                results = batchResults;
            },
            [&](std::size_t completed, std::size_t total)
            {
                progress.emplace_back(completed, total);
            });

        state.complete(2, input[2]);

        expect(completionCallbacks == 0, "third result alone is not enough");
        expect(progress.size() == 1, "first completion reports progress");

        pings::PingResult duplicate;
        duplicate.host = "must not replace third";
        state.complete(2, duplicate);

        expect(progress.size() == 1, "duplicate does not advance progress");
        expect(completionCallbacks == 0, "duplicate does not finish batch");

        state.complete(1, input[1]);

        expect(completionCallbacks == 0, "two unique results are not enough");

        state.complete(0, input[0]);

        const std::vector<std::pair<std::size_t, std::size_t>>
            expectedProgress{{1, 3}, {2, 3}, {3, 3}};

        expect(progress == expectedProgress, "progress counts unique results");
        expect(completionCallbacks == 1, "all unique results complete batch once");
        expect(results.size() == input.size(), "all results are returned");

        if (results.size() == input.size())
        {
            for (std::size_t i = 0; i < input.size(); ++i)
            {
                expect(
                    results[i].host == input[i].host,
                    "results preserve input positions");
                expect(
                    results[i].status == input[i].status,
                    "result status is preserved");
            }
        }

        state.complete(0, duplicate);
        state.complete(1, duplicate);
        state.complete(2, duplicate);

        expect(completionCallbacks == 1, "late duplicates do not repeat completion");
        expect(progress == expectedProgress, "late duplicates do not report progress");
    }
}

int main()
{
    try
    {
        testReverseCompletionOrder();
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