#ifndef READ_HPP_
#define READ_HPP_

#include <algorithm>
#include <cstdint>

namespace readutil
{
    const uint8_t* read8(uint8_t& v, const uint8_t* data);
    const uint8_t* read16(uint16_t& v, const uint8_t* data);
    const uint8_t* read24(uint32_t& v, const uint8_t* data);
    const uint8_t* read32(uint32_t& v, const uint8_t* data);

    template <size_t N>
    const uint8_t* read(uint8_t (&v)[N], const uint8_t* data)
    {
        std::copy_n(data, N, v);
        return data + N;
    }
}

#endif
