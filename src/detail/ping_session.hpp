#pragma once
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include <boost/asio/ip/icmp.hpp>
#include <boost/asio/steady_timer.hpp>

#include <pings/ping_options.hpp>
#include <pings/ping_result.hpp>
#include <pings/ping_types.hpp>

namespace pings::detail
{
    enum class IpFamily
    {
        Unknown,
        IPv4,
        IPv6
    };

    // Внутреннее состояние одной ping-операции.
    struct PingSession
    {
        std::string host;
        boost::asio::ip::icmp::endpoint destination;
        IpFamily ipFamily{IpFamily::Unknown};

        PingOptions options;

        std::chrono::steady_clock::time_point sendTime;
        std::unique_ptr<boost::asio::steady_timer> timeoutTimer;
        std::unique_ptr<boost::asio::steady_timer> resolveTimer;

        std::uint16_t sequenceNumber;
        bool completed{false};

        bool requestSent{false};

        PingCallback callback;
    };
} // namespace pings::detail
