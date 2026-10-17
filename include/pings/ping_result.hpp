#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

#include <boost/asio/ip/address.hpp>
#include <boost/system/error_code.hpp>

namespace pings
{
    enum class PingStatus
    {
        Success,
        Timeout,

        ResolveError,
        ResolveTimeout,

        SendError,
        Cancelled,
        Stopped,
        InternalError,
        InvalidOptions
    };

    struct PingResult
    {
        std::string host;
        std::optional<boost::asio::ip::address> address;

        std::optional<std::chrono::milliseconds> latency{};

        PingStatus status{PingStatus::InternalError};
        boost::system::error_code error;

        bool requestSent{false};

        [[nodiscard]]
        bool success() const noexcept
        {
            return status == PingStatus::Success;
        }
    };

    constexpr std::string_view toString(PingStatus status) noexcept
    {
        switch (status)
        {
        case PingStatus::Success:
            return "success";

        case PingStatus::Timeout:
            return "timeout";

        case PingStatus::ResolveError:
            return "resolve_error";

        case PingStatus::ResolveTimeout:
            return "resolve_timeout";

        case PingStatus::SendError:
            return "send_error";

        case PingStatus::Cancelled:
            return "cancelled";

        case PingStatus::Stopped:
            return "stopped";

        case PingStatus::InternalError:
            return "internal_error";

        case PingStatus::InvalidOptions:
            return "invalid_options";
        }

        return "unknown";
    }
}
