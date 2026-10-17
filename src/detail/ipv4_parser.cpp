#include <ipv4_parser.hpp>
#include <icmp_codec.hpp>
#include <network_bytes.hpp>

namespace pings::ipv4
{
    std::optional<PacketView> parse(const std::uint8_t *data, std::size_t size)
    {
        constexpr std::size_t MinHeaderSize = 20;

        if (size < MinHeaderSize)
        {
            return std::nullopt;
        }

        const auto version = static_cast<std::uint8_t>(data[0] >> 4);

        if (version != 4)
        {
            return std::nullopt;
        }

        const std::size_t headerLength = static_cast<std::size_t>((data[0] & 0x0F) * 4);

        if (headerLength < MinHeaderSize || headerLength > size)
        {
            return std::nullopt;
        }

        const std::uint16_t totalLength = netbytes::readUint16BigEndian(data + 2);

        if (totalLength < headerLength || totalLength > size)
        {
            return std::nullopt;
        }

        const std::uint16_t flagsAndFragmentOffset = netbytes::readUint16BigEndian(data + 6);

        constexpr std::uint16_t MoreFragmentsFlag = 0x2000;
        constexpr std::uint16_t FragmentOffsetMask = 0x1FFF;

        const bool moreFragments = (flagsAndFragmentOffset & MoreFragmentsFlag) != 0;

        const std::uint16_t fragmentOffset = flagsAndFragmentOffset & FragmentOffsetMask;

        if (moreFragments || fragmentOffset != 0)
        {
            return std::nullopt;
        }

        boost::asio::ip::address_v4::bytes_type sourceBytes{
            data[12],
            data[13],
            data[14],
            data[15]};

        boost::asio::ip::address_v4::bytes_type destinationBytes{
            data[16],
            data[17],
            data[18],
            data[19]};

        PacketView packet;

        packet.payload = data + headerLength;

        packet.payloadSize = totalLength - headerLength;

        packet.sourceAddress = boost::asio::ip::address_v4(sourceBytes);
        packet.destinationAddress = boost::asio::ip::address_v4(destinationBytes);

        packet.protocol = data[9];

        return packet;
    }
}