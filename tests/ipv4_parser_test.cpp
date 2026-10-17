#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include <ipv4_parser.hpp>

namespace
{
    int checks = 0;
    int failures = 0;

    void expect(bool condition, std::string_view name)
    {
        ++checks;

        if (!condition)
        {
            ++failures;
            std::cerr << "FAIL: " << name << '\n';
        }
    }
}

int main()
{
    using pings::ipv4::parse;

    // IPv4: заголовок 20 байт, весь пакет 28 байт.
    // Источник: 192.0.2.1, получатель: 198.51.100.2.
    // Protocol=1. После заголовка — восемь байт ICMP.
    const std::array<std::uint8_t, 28> valid{
        0x45, 0x00, 0x00, 0x1C,
        0x00, 0x00, 0x00, 0x00,
        0x40, 0x01, 0x8E, 0xAA,
        0xC0, 0x00, 0x02, 0x01,
        0xC6, 0x33, 0x64, 0x02,

        0x00, 0x00, 0x97, 0x53,
        0x12, 0x34, 0x56, 0x78};

    const auto packet = parse(valid.data(), valid.size());

    const boost::asio::ip::address_v4 source{
        boost::asio::ip::address_v4::bytes_type{192, 0, 2, 1}};

    const boost::asio::ip::address_v4 destination{
        boost::asio::ip::address_v4::bytes_type{198, 51, 100, 2}};

    expect(
        packet &&
            packet->payload == valid.data() + 20 &&
            packet->payloadSize == 8 &&
            packet->sourceAddress == source &&
            packet->destinationAddress == destination &&
            packet->protocol == 1,
        "valid packet: fields and payload");

    expect(
        !parse(valid.data(), 19),
        "buffer shorter than minimum header");

    {
        auto bytes = valid;
        bytes[0] = 0x65;

        expect(
            !parse(bytes.data(), bytes.size()),
            "wrong IP version");
    }

    {
        auto bytes = valid;
        bytes[0] = 0x44; // IHL=4: всего 16 байт.

        expect(
            !parse(bytes.data(), bytes.size()),
            "IHL below minimum");
    }

    {
        auto bytes = valid;
        bytes[0] = 0x4F; // IHL=15: 60 байт при буфере 28.

        expect(
            !parse(bytes.data(), bytes.size()),
            "IHL exceeds available buffer");
    }

    {
        auto bytes = valid;
        bytes[3] = 19; // Total Length меньше заголовка.

        expect(
            !parse(bytes.data(), bytes.size()),
            "total length below header length");
    }

    {
        auto bytes = valid;
        bytes[3] = 29; // Заявлено больше, чем получено.

        expect(
            !parse(bytes.data(), bytes.size()),
            "total length exceeds available buffer");
    }

    {
        // IHL=6: заголовок 24 байта, включая четыре байта options.
        const std::array<std::uint8_t, 32> bytes{
            0x46, 0x00, 0x00, 0x20,
            0x00, 0x00, 0x00, 0x00,
            0x40, 0x01, 0x8D, 0xA6,
            0xC0, 0x00, 0x02, 0x01,
            0xC6, 0x33, 0x64, 0x02,
            0x00, 0x00, 0x00, 0x00,

            0x00, 0x00, 0x97, 0x53,
            0x12, 0x34, 0x56, 0x78};

        const auto parsed = parse(bytes.data(), bytes.size());

        expect(
            parsed &&
                parsed->payload == bytes.data() + 24 &&
                parsed->payloadSize == 8,
            "header with options: payload offset");
    }

    {
        auto bytes = valid;
        bytes[6] = 0x20; // More Fragments, offset=0.

        expect(
            !parse(bytes.data(), bytes.size()),
            "first fragment with MF flag");
    }

    {
        auto bytes = valid;
        bytes[7] = 0x01; // Fragment Offset != 0, MF=0.

        expect(
            !parse(bytes.data(), bytes.size()),
            "last fragment with nonzero offset");
    }

    {
        auto bytes = valid;
        bytes[6] = 0x40; // Don't Fragment, offset=0, MF=0.

        expect(
            parse(bytes.data(), bytes.size()).has_value(),
            "DF flag on an unfragmented packet");
    }

    {
        auto bytes = valid;
        bytes[3] = 24; // Буфер — 28 байт, пакет — 24 байта.

        const auto parsed = parse(bytes.data(), bytes.size());

        expect(
            parsed &&
                parsed->payload == bytes.data() + 20 &&
                parsed->payloadSize == 4,
            "payload limited by total length");
    }

    {
        auto bytes = valid;
        bytes[3] = 20; // Total Length равен длине заголовка.

        const auto parsed = parse(bytes.data(), bytes.size());

        expect(
            parsed &&
                parsed->payload == bytes.data() + 20 &&
                parsed->payloadSize == 0,
            "packet with empty payload");
    }

    std::cout
        << "Checks: " << checks
        << ", failures: " << failures << '\n';

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}