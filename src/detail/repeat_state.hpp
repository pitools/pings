#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include <boost/asio/steady_timer.hpp>

#include <pings/ping_handle.hpp>
#include <pings/repeat_options.hpp>
#include <pings/repeat_types.hpp>
#include <pings/ping_statistics.hpp>

namespace pings::detail
{
    struct RepeatState
    {
        std::string host;
        RepeatOptions options;

        std::size_t completed{};

        bool cancelled{false};
        bool finished{false};

        PingHandle currentPing;

        std::unique_ptr<
            boost::asio::steady_timer>
            intervalTimer;

        RepeatResultCallback callback;
        RepeatCompletionCallback completionCallback;

        PingStatistics statistics;
        std::chrono::milliseconds totalLatency{};
    };
}
