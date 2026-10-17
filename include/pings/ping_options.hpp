#pragma once

#include <chrono>
#include <cstddef>

namespace pings
{
    enum class AddressFamily
    {
        Any,
        IPv4,
        IPv6
    };

    struct PingOptions
    {
        std::chrono::milliseconds replyTimeout{1000};
        std::chrono::milliseconds resolveTimeout{3000};

        std::size_t payloadSize{32};

        AddressFamily addressFamily{AddressFamily::Any};
    };

    inline constexpr auto MinReplyTimeout =
        std::chrono::milliseconds{1};

    inline constexpr auto MaxReplyTimeout =
        std::chrono::minutes{1};

    inline constexpr auto MinResolveTimeout =
        std::chrono::milliseconds{1};

    inline constexpr auto MaxResolveTimeout =
        std::chrono::minutes{1};

    inline constexpr std::size_t MaxPayloadSize = 1400;

    [[nodiscard]]
    bool isValid(const PingOptions &options) noexcept;

}