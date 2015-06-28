#ifndef SHOW_HPP_
#define SHOW_HPP_

#include <string>
#include <ostream>
#include <cstddef>
#include "bootsector.hpp"

namespace showutil
{
    class FixedLength
    {
        std::string s_;
        size_t length_;

        friend std::ostream& operator <<(std::ostream& os, const FixedLength& f);
    public:
        FixedLength(std::string s, size_t length)
            : s_(s), length_(length)
        {}
    };

    std::ostream& operator <<(std::ostream& os, const FixedLength& f);

    template <typename S, typename L>
    FixedLength fixlen(S&& s, L&& l)
    {
        return FixedLength(std::forward<S>(s), std::forward<L>(l));
    }

    class HexBytes
    {
        std::basic_string<uint8_t> s_;

        friend std::ostream& operator <<(std::ostream& os, const HexBytes& b);
    public:
        template <size_t N>
        HexBytes(const uint8_t (&v)[N])
            : s_(v, N)
        {}
    };

    std::ostream& operator <<(std::ostream& os, const HexBytes& b);

    template <typename T>
    HexBytes bytes(T&& v)
    {
        return HexBytes(std::forward<T>(v));
    }

    std::ostream& operator <<(std::ostream& os, uint8_t v);
}

#endif
