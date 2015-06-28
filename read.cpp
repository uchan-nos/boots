#include "read.hpp"

#include <cstddef>

namespace readutil
{
    const uint8_t* read8(uint8_t& v, const uint8_t* data)
    {
        v = data[0];
        return data + 1;
    }

    const uint8_t* read16(uint16_t& v, const uint8_t* data)
    {
        v = (data[1]) << 8 | data[0];
        return data + 2;
    }

    const uint8_t* read24(uint32_t& v, const uint8_t* data)
    {
        v = (data[2] << 16) | (data[1]) << 8 | data[0];
        return data + 3;
    }

    const uint8_t* read32(uint32_t& v, const uint8_t* data)
    {
        v = (data[3] << 24) | (data[2] << 16) | (data[1]) << 8 | data[0];
        return data + 4;
    }
}
