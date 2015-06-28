#include "bootsector.hpp"

using namespace std;

BootSector::~BootSector()
{}

BootSector::Type infer(const std::array<uint8_t, 512>& data)
{
    if (data[510] == 0x55 && data[511] == 0xaa)
    {
        if ((data[0] == 0xeb && data[2] == 0x90) || data[0] == 0xe9)
        {
            // BS_jmpBoot is valid.
            return BootSector::Type::kPbrFat;
        }
        else if (data[0x01be] == 0x00 || data[0x01be] == 0x80)
        {
            // Active flag is valid.
            return BootSector::Type::kMbr;
        }
    }
    return BootSector::Type::kUnknown;
}

std::unique_ptr<BootSector> make_bs(
        const std::array<uint8_t, 512>& data, BootSector::Type type)
{
    switch (type)
    {
    case BootSector::Type::kMbr:
        return unique_ptr<Mbr>(new Mbr(data));
    case BootSector::Type::kPbrFat:
        return unique_ptr<PbrFat>(new PbrFat(data));
    default:
        return unique_ptr<BootSector>();
    }
}

unsigned int cylinder(const uint8_t (&chs)[3])
{
    return (static_cast<unsigned int>(chs[1] & 0xc0u) << 2) | chs[2];
}

unsigned int head(const uint8_t (&chs)[3])
{
    return chs[0];
}

unsigned int sector(const uint8_t (&chs)[3])
{
    return chs[1] & 0x3fu;
}
