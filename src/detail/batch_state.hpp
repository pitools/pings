#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include <pings/batch_types.hpp>
#include <pings/ping_result.hpp>

namespace pings::detail
{
    class BatchState
    {
    public:
        BatchState(std::size_t size, BatchCompletionCallback callback, BatchProgressCallback progressCallback = {});

        void complete(std::size_t index, const PingResult &result);

    private:
        std::vector<PingResult> results_;
        std::vector<bool> completed_;

        std::size_t remaining_;
        std::size_t total_{};

        BatchCompletionCallback completionCallback_;
        BatchProgressCallback progressCallback_;
    };
} // namespace pings::detail
