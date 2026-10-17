#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

namespace pings::icmpv6
{
    constexpr std::uint8_t EchoRequestType = 128;
    constexpr std::uint8_t EchoReplyType = 129;
    constexpr std::uint8_t EchoCode = 0;

    constexpr std::size_t EchoHeaderSize = 8;

    struct EchoReply
    {
        std::uint16_t identifier{};
        std::uint16_t sequence{};
    };

    // data указывает на начало ICMPv6-сообщения, без IPv6-заголовка.
    // Проверяет структуру Echo Reply; checksum здесь не проверяется.
    std::optional<EchoReply> parseEchoReply(
        const std::uint8_t *data,
        std::size_t size);

    // data должен указывать на минимум EchoHeaderSize доступных байт.
    // Поле checksum обнуляется; его заполнение относится к транспорту.
    void writeEchoRequestHeader(
        std::uint8_t *data,
        std::uint16_t identifier,
        std::uint16_t sequence);
}