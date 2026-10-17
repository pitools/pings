#include <icmpv6_codec.hpp>

#include <network_bytes.hpp>

namespace pings::icmpv6
{
    void writeEchoRequestHeader(
        std::uint8_t *data,
        std::uint16_t identifier,
        std::uint16_t sequence)
    {
        data[0] = EchoRequestType;
        data[1] = EchoCode;

        netbytes::writeUint16BigEndian(data + 2, 0);
        netbytes::writeUint16BigEndian(data + 4, identifier);
        netbytes::writeUint16BigEndian(data + 6, sequence);
    }

    std::optional<EchoReply> parseEchoReply(
        const std::uint8_t *data,
        std::size_t size)
    {
        if (data == nullptr || size < EchoHeaderSize)
        {
            return std::nullopt;
        }

        if (data[0] != EchoReplyType || data[1] != EchoCode)
        {
            return std::nullopt;
        }

        return EchoReply{
            netbytes::readUint16BigEndian(data + 4),
            netbytes::readUint16BigEndian(data + 6)};
    }
}