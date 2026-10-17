#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include <icmpv6_codec.hpp>

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
    using pings::icmpv6::parseEchoReply;
    using pings::icmpv6::writeEchoRequestHeader;

    // Независимый ожидаемый wire format.
    const std::array<std::uint8_t, 8> expectedRequest{
        0x80, 0x00, 0x00, 0x00,
        0x12, 0x34, 0xAB, 0xCD};

    std::array<std::uint8_t, 8> request{};
    request.fill(0xFF);

    writeEchoRequestHeader(request.data(), 0x1234, 0xABCD);

    expect(
        request == expectedRequest,
        "request: type, code, zero checksum and byte order");

    // Искусственный ответ для проверки структуры.
    // Значение checksum здесь не проверяется.
    const std::array<std::uint8_t, 8> replyBytes{
        0x81, 0x00, 0x55, 0xAA,
        0x12, 0x34, 0xAB, 0xCD};

    const auto reply =
        parseEchoReply(replyBytes.data(), replyBytes.size());

    expect(
        reply &&
            reply->identifier == 0x1234 &&
            reply->sequence == 0xABCD,
        "reply: identifier and sequence byte order");

    for (std::size_t size = 0; size < replyBytes.size(); ++size)
    {
        expect(
            !parseEchoReply(replyBytes.data(), size),
            "truncated reply");
    }

    expect(
        !parseEchoReply(nullptr, 8),
        "null pointer");

    expect(
        !parseEchoReply(request.data(), request.size()),
        "echo request is not a reply");

    auto wrongCode = replyBytes;
    wrongCode[1] = 1;

    expect(
        !parseEchoReply(wrongCode.data(), wrongCode.size()),
        "nonzero reply code");

    const std::array<std::uint8_t, 11> withPayload{
        0x81, 0x00, 0x55, 0xAA,
        0x12, 0x34, 0xAB, 0xCD,
        0x42, 0x43, 0x44};

    const auto payloadReply =
        parseEchoReply(withPayload.data(), withPayload.size());

    expect(
        payloadReply &&
            payloadReply->identifier == 0x1234 &&
            payloadReply->sequence == 0xABCD,
        "reply with payload");

    std::cout
        << "Checks: " << checks
        << ", failures: " << failures << '\n';

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}