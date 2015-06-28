#include "show.hpp"

#include <boost/format.hpp>
using namespace std;
using namespace boost;

namespace showutil
{
    ostream& operator <<(ostream& os, const FixedLength& f)
    {
        os << f.s_;
        for (size_t i = f.s_.length(); i < f.length_; ++i)
        {
            os.put(' ');
        }
        return os;
    }

    ostream& operator <<(ostream& os, const HexBytes& b)
    {
        auto pat = format("%02x");
        os << pat % (int)b.s_[0];
        for (size_t i = 1; i < b.s_.length(); ++i)
        {
            os << ' ' << pat % (int)b.s_[i];
        }
        return os;
    }

    ostream& operator <<(ostream& os, uint8_t v)
    {
        os << (int)v;
        return os;
    }
}
