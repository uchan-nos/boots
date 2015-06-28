#include "bootsector.hpp"

#include <boost/format.hpp>
#include "read.hpp"
#include "show.hpp"

using namespace std;
using namespace boost;
using namespace readutil;
using namespace showutil;

Mbr::Mbr(const uint8_t* data)
{
    data = read(this->boot_code, data);

    for (int i = 0; i < 4; ++i)
    {
        data = read8(this->part_table[i].active, data);
        data = read(this->part_table[i].chs_start, data);
        data = read8(this->part_table[i].type, data);
        data = read(this->part_table[i].chs_end, data);
        data = read32(this->part_table[i].lba_start, data);
        data = read32(this->part_table[i].size, data);
    }

    data = read(this->last_signature, data);
}

void Mbr::print_info(std::ostream& os) const
{
    for (int i = 0; i < 4; ++i)
    {
        const auto& p = part_table[i];

        os << "Partition " << i << endl;
        os << fixlen("  Active flag:", 20)
            << format("%02x") % (int)p.active << endl;
        os << fixlen("  CHS Start:", 20)
            << "C:" << cylinder(p.chs_start)
            << ", H:" << head(p.chs_start)
            << ", S:" << sector(p.chs_start)
            << " (" << bytes(p.chs_start) << ")" << endl;
        os << fixlen("  Partition Type:", 20) << p.type << endl;
        os << fixlen("  CHS End:", 20)
            << "C:" << cylinder(p.chs_end)
            << ", H:" << head(p.chs_end)
            << ", S:" << sector(p.chs_end)
            << " (" << bytes(p.chs_end) << ")" << endl;
        os << fixlen("  LBA Start:", 20)
            << format("%08x") % p.lba_start << endl;
        os << fixlen("  Number of Sectors:", 20)
            << format("%08x") % p.size << endl;
    }
}

void Mbr::print_asm(std::ostream& os) const
{
}
