#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include <boost/asio/ip/address_v4.hpp>

namespace pings::ipv4
{
    struct PacketView
    {
        const std::uint8_t *payload{};
        std::size_t payloadSize{};

        boost::asio::ip::address_v4 sourceAddress;
        boost::asio::ip::address_v4 destinationAddress;

        std::uint8_t protocol{};
    };

    std::optional<PacketView> parse(const std::uint8_t *data, std::size_t size);
}
