#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include <icmp_codec.hpp>

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
    using pings::icmp::parseEchoReply;

    // type=0, code=0, id=0x1234, sequence=0x5678.
    // 0x1234 + 0x5678 = 0x68AC; checksum = 0x9753.
    const std::array<std::uint8_t, 8> valid{
        0x00, 0x00, 0x97, 0x53,
        0x12, 0x34, 0x56, 0x78};

    const auto reply = parseEchoReply(valid.data(), valid.size());

    expect(
        reply &&
            reply->identifier == 0x1234 &&
            reply->sequence == 0x5678,
        "valid reply: identifier and sequence");

    // Все длины меньше минимального заголовка: от 0 до 7.
    for (std::size_t size = 0; size < valid.size(); ++size)
    {
        const auto parsed = parseEchoReply(valid.data(), size);

        expect(!parsed, "truncated header");

        if (parsed)
        {
            std::cerr << "  accepted size=" << size << '\n';
        }
    }

    auto badChecksum = valid;
    badChecksum[2] ^= 0x01;

    expect(
        !parseEchoReply(badChecksum.data(), badChecksum.size()),
        "damaged checksum");

    // Echo Request: type=8, checksum пересчитана заранее.
    const std::array<std::uint8_t, 8> wrongType{
        0x08, 0x00, 0x8F, 0x53,
        0x12, 0x34, 0x56, 0x78};

    expect(
        !parseEchoReply(wrongType.data(), wrongType.size()),
        "wrong type with valid checksum");

    // Неверный code=1, но checksum корректна.
    const std::array<std::uint8_t, 8> wrongCode{
        0x00, 0x01, 0x97, 0x52,
        0x12, 0x34, 0x56, 0x78};

    expect(
        !parseEchoReply(wrongCode.data(), wrongCode.size()),
        "wrong code with valid checksum");

    // Один байт payload добавляет к сумме 0x0100.
    // 0x68AC + 0x0100 = 0x69AC; checksum = 0x9653.
    auto withPayload = std::array<std::uint8_t, 9>{
        0x00, 0x00, 0x96, 0x53,
        0x12, 0x34, 0x56, 0x78,
        0x01};

    const auto payloadReply =
        parseEchoReply(withPayload.data(), withPayload.size());

    expect(
        payloadReply &&
            payloadReply->identifier == 0x1234 &&
            payloadReply->sequence == 0x5678,
        "valid reply with odd-length payload");

    // Меняем payload, оставляя прежнюю checksum.
    withPayload[8] = 0x00;

    expect(
        !parseEchoReply(withPayload.data(), withPayload.size()),
        "damaged payload");

    std::cout
        << "Checks: " << checks
        << ", failures: " << failures << '\n';

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}