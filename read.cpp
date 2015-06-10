#include "read.hpp"

#include <algorithm>
#include <cstddef>
using namespace std;

namespace
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

    template <size_t N>
    const uint8_t* read(uint8_t (&v)[N], const uint8_t* data)
    {
        copy_n(data, N, v);
        return data + N;
    }
}

void read(Mbr& mbr, const uint8_t* data)
{
}

void read(PbrFat& pbr, const uint8_t* data)
{
    data = read(pbr.BS_jmpBoot, data);
    data = read(pbr.BS_OEMName, data);
    data = read16(pbr.BPB_BytsPerSec, data);
    data = read8(pbr.BPB_SecPerClus, data);
    data = read16(pbr.BPB_RsvdSecCnt, data);
    data = read8(pbr.BPB_NumFATs, data);
    data = read16(pbr.BPB_RootEntCnt, data);
    data = read16(pbr.BPB_TotSec16, data);
    data = read8(pbr.BPB_Media, data);
    data = read16(pbr.BPB_FATSz16, data);
    data = read16(pbr.BPB_SecPerTrk, data);
    data = read16(pbr.BPB_NumHeads, data);
    data = read32(pbr.BPB_HiddSec, data);
    data = read32(pbr.BPB_TotSec32, data);

    const uint8_t* data0 = data;

    data = read8(pbr.fat12_16.BS_DrvNum, data);
    data = read8(pbr.fat12_16.BS_Reserved1, data);
    data = read8(pbr.fat12_16.BS_BootSig, data);
    data = read32(pbr.fat12_16.BS_VolID, data);
    data = read(pbr.fat12_16.BS_VolLab, data);
    data = read(pbr.fat12_16.BS_FilSysType, data);
    data = read(pbr.fat12_16.boot_program, data);

    data = data0;

    data = read32(pbr.fat32.BPB_FATSz32, data);
    data = read16(pbr.fat32.BPB_ExtFlags, data);
    data = read(pbr.fat32.BPB_FSVer, data);
    data = read32(pbr.fat32.BPB_RootClus, data);
    data = read16(pbr.fat32.BPB_FSInfo, data);
    data = read16(pbr.fat32.BPB_BkBootSec, data);
    data = read(pbr.fat32.BPB_Reserved, data);
    data = read8(pbr.fat32.BS_DrvNum, data);
    data = read8(pbr.fat32.BS_Reserved1, data);
    data = read8(pbr.fat32.BS_BootSig, data);
    data = read32(pbr.fat32.BS_VolID, data);
    data = read(pbr.fat32.BS_VolLab, data);
    data = read(pbr.fat32.BS_FilSysType, data);
    data = read(pbr.fat32.boot_program, data);

    data = read(pbr.last_signature, data);
}
