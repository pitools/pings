#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include <icmp_codec.hpp>

namespace
{
    int checks = 0;
    int failures = 0;

    void expectChecksum(
        std::string_view name,
        std::uint16_t actual,
        std::uint16_t expected)
    {
        ++checks;

        if (actual != expected)
        {
            ++failures;

            std::cerr
                << "FAIL: " << name
                << ", expected=0x" << std::hex << expected
                << ", actual=0x" << actual
                << std::dec << '\n';
        }
    }
}

int main()
{
    using pings::icmp::calculateChecksum;

    // Пример из RFC 1071: сумма после переноса = 0xDDF2.
    // Инверсия даёт 0x220D.
    const std::array<std::uint8_t, 8> reference{
        0x00, 0x01, 0xF2, 0x03,
        0xF4, 0xF5, 0xF6, 0xF7};

    expectChecksum(
        "RFC 1071 example",
        calculateChecksum(reference.data(), reference.size()),
        0x220D);

    // 0x0102 + 0x0300 = 0x0402; инверсия = 0xFBFD.
    const std::array<std::uint8_t, 3> odd{
        0x01, 0x02, 0x03};

    expectChecksum(
        "odd number of bytes",
        calculateChecksum(odd.data(), odd.size()),
        0xFBFD);

    // 0xFFFF + 0x0001 = 0x10000.
    // После переноса: 0x0001; инверсия = 0xFFFE.
    const std::array<std::uint8_t, 4> carry{
        0xFF, 0xFF, 0x00, 0x01};

    expectChecksum(
        "end-around carry",
        calculateChecksum(carry.data(), carry.size()),
        0xFFFE);

    // Ещё один перенос нужен после первого сворачивания суммы.
    const std::array<std::uint8_t, 6> repeatedCarry{
        0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01};

    expectChecksum(
        "repeated carry",
        calculateChecksum(repeatedCarry.data(), repeatedCarry.size()),
        0xFFFE);

    // Echo Reply без payload:
    // type=0, code=0, checksum=0xEDCA, id=0x1234, sequence=1.
    auto reply = std::array<std::uint8_t, 8>{
        0x00, 0x00, 0xED, 0xCA,
        0x12, 0x34, 0x00, 0x01};

    expectChecksum(
        "valid packet including checksum",
        calculateChecksum(reply.data(), reply.size()),
        0x0000);

    // Меняем sequence с 1 на 0, сохраняя старую checksum.
    reply[7] = 0x00;

    expectChecksum(
        "one-bit corruption",
        calculateChecksum(reply.data(), reply.size()),
        0x0001);

    std::cout
        << "Checks: " << checks
        << ", failures: " << failures << '\n';

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}