#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

namespace pings::netbytes
{
    inline std::uint16_t readUint16BigEndian(const std::uint8_t *data)
    {
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(data[0]) << 8) |
            static_cast<std::uint16_t>(data[1]));
    }

    inline void writeUint16BigEndian(uint8_t *data, std::uint16_t value)
    {
        data[0] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
        data[1] = static_cast<std::uint8_t>(value & 0xFF);
    }
}
