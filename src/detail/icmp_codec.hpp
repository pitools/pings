#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

namespace pings::icmp
{
    struct EchoReply
    {
        std::uint16_t identifier{};
        std::uint16_t sequence{};
    };

    constexpr std::uint8_t EchoReplyType = 0;
    constexpr std::uint8_t EchoRequestType = 8;
    constexpr std::uint8_t EchoCode = 0;

    constexpr std::size_t TypeOffset = 0;
    constexpr std::size_t CodeOffset = 1;
    constexpr std::size_t ChecksumOffset = 2;
    constexpr std::size_t IdentifierOffset = 4;
    constexpr std::size_t SequenceOffset = 6;
    constexpr std::size_t EchoHeaderSize = 8;

    // Вспомогательная функция для расчёта контрольной суммы ICMP
    std::uint16_t calculateChecksum(const void *data, size_t size);

    void writeEchoRequestHeader(uint8_t *data, std::uint16_t identifier, std::uint16_t sequence);

    std::optional<EchoReply> parseEchoReply(const std::uint8_t *data, std::size_t size);
}