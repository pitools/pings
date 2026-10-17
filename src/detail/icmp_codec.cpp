#include <icmp_codec.hpp>

namespace pings::icmp
{
    std::uint16_t calculateChecksum(const void *data, std::size_t size)
    {
        const auto *bytes = static_cast<const std::uint8_t *>(data);

        std::uint32_t sum = 0;

        // Каждые два байта пакета интерпретируем
        // как одно 16-битное слово в network byte order
        while (size >= 2)
        {
            const std::uint16_t word = static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[0])) << 8 | static_cast<std::uint16_t>(bytes[1]));

            sum += word;
            bytes += 2;
            size -= 2;
        }
        // Если количество байтов нечётное,
        // последний байт является старшим байтом
        // последнего 16-битного слова.
        if (size == 1)
        {
            sum += static_cast<std::uint16_t>(bytes[0]) << 8;
        }

        // Складываем переносы обратно в младшие 16 бит.
        while (sum >> 16)
        {
            sum =
                (sum & 0xFFFF) +
                (sum >> 16);
        }

        // size_t count = size / 2;

        return static_cast<std::uint16_t>(~sum);
    }

    std::uint16_t readUint16BigEndian(const std::uint8_t *data)
    {
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(data[0]) << 8) |
            static_cast<std::uint16_t>(data[1]));
    }

    void writeUint16BigEndian(std::uint8_t *data, std::uint16_t value)
    {
        data[0] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
        data[1] = static_cast<std::uint8_t>(value & 0xFF);
    }

    // Сериализует ICMP Echo Request header напрямую в wire format
    void writeEchoRequestHeader(
        std::uint8_t *data,
        std::uint16_t identifier,
        std::uint16_t sequence)
    {
        data[TypeOffset] = EchoRequestType;
        data[CodeOffset] = EchoCode;

        writeUint16BigEndian(data + ChecksumOffset, 0);

        writeUint16BigEndian(data + IdentifierOffset, identifier);

        writeUint16BigEndian(data + SequenceOffset, sequence);
    }

    std::optional<EchoReply> parseEchoReply(const std::uint8_t *data, size_t size)
    {
        if (size < EchoHeaderSize)
        {
            return std::nullopt;
        }
        // Для корректного ICMP-сообщения checksum по всему сообщению,
        // включая уже заполненное поле checksum, должна дать 0.
        if (calculateChecksum(data, size) != 0)
        {
            return std::nullopt;
        }

        if (data[TypeOffset] != EchoReplyType ||
            data[CodeOffset] != EchoCode)
        {
            return std::nullopt;
        }

        return EchoReply{
            readUint16BigEndian(data + IdentifierOffset),
            readUint16BigEndian(data + SequenceOffset)};
    }

}