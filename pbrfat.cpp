#include "bootsector.hpp"

#include <boost/format.hpp>
#include "read.hpp"
#include "show.hpp"

using namespace std;
using namespace boost;
using namespace readutil;
using namespace showutil;

PbrFat::PbrFat(const uint8_t* data)
{
    data = read(this->BS_jmpBoot, data);
    data = read(this->BS_OEMName, data);
    data = read16(this->BPB_BytsPerSec, data);
    data = read8(this->BPB_SecPerClus, data);
    data = read16(this->BPB_RsvdSecCnt, data);
    data = read8(this->BPB_NumFATs, data);
    data = read16(this->BPB_RootEntCnt, data);
    data = read16(this->BPB_TotSec16, data);
    data = read8(this->BPB_Media, data);
    data = read16(this->BPB_FATSz16, data);
    data = read16(this->BPB_SecPerTrk, data);
    data = read16(this->BPB_NumHeads, data);
    data = read32(this->BPB_HiddSec, data);
    data = read32(this->BPB_TotSec32, data);

    const uint8_t* data0 = data;

    data = read8(this->fat12_16.BS_DrvNum, data);
    data = read8(this->fat12_16.BS_Reserved1, data);
    data = read8(this->fat12_16.BS_BootSig, data);
    data = read32(this->fat12_16.BS_VolID, data);
    data = read(this->fat12_16.BS_VolLab, data);
    data = read(this->fat12_16.BS_FilSysType, data);
    data = read(this->fat12_16.boot_program, data);

    data = data0;

    data = read32(this->fat32.BPB_FATSz32, data);
    data = read16(this->fat32.BPB_ExtFlags, data);
    data = read(this->fat32.BPB_FSVer, data);
    data = read32(this->fat32.BPB_RootClus, data);
    data = read16(this->fat32.BPB_FSInfo, data);
    data = read16(this->fat32.BPB_BkBootSec, data);
    data = read(this->fat32.BPB_Reserved, data);
    data = read8(this->fat32.BS_DrvNum, data);
    data = read8(this->fat32.BS_Reserved1, data);
    data = read8(this->fat32.BS_BootSig, data);
    data = read32(this->fat32.BS_VolID, data);
    data = read(this->fat32.BS_VolLab, data);
    data = read(this->fat32.BS_FilSysType, data);
    data = read(this->fat32.boot_program, data);

    data = read(this->last_signature, data);
}

void PbrFat::print_info(std::ostream& os) const
{
    // Determin FAT type
    size_t root_dir_sectors =
        (BPB_RootEntCnt * 32 + BPB_BytsPerSec - 1) / BPB_BytsPerSec;

    const size_t fat_size =
        BPB_FATSz16 != 0 ? BPB_FATSz16 : fat32.BPB_FATSz32;

    const size_t total_sectors =
        BPB_TotSec16 != 0 ? BPB_TotSec16 : BPB_TotSec32;

    const size_t data_sectors = total_sectors
        - (BPB_RsvdSecCnt + BPB_NumFATs * fat_size + root_dir_sectors);

    const size_t count_of_clusters = data_sectors / BPB_SecPerClus;

    const int fat_type =
        count_of_clusters < 4085 ? 12
        : count_of_clusters < 65525 ? 16
        : 32;

    os << "Calculated fat type is FAT" << fat_type << endl;

    // Check file system string
    string fs_str_12_16(reinterpret_cast<const char *>(fat12_16.BS_FilSysType), 8);
    string fs_str_32(reinterpret_cast<const char *>(fat32.BS_FilSysType), 8);

    if ((fs_str_12_16 == "FAT12   "
        || fs_str_12_16 == "FAT16   "
        || fs_str_12_16 == "FAT     ") && fat_type == 32)
    {
        os << "But there is a string indicating file system is FAT12 or 16." << endl;
    }

    if (fs_str_32 == "FAT32   " && (fat_type == 12 || fat_type == 16))
    {
        os << "But there is a string indicating file system is FAT32." << endl;
    }

    os << fixlen("BS_jmpBoot:", 20) << bytes(BS_jmpBoot) << endl;
    os << fixlen("BS_OEMName:", 20)
        << string(reinterpret_cast<const char *>(BS_OEMName), 8) << endl;
    os << fixlen("BPB_BytsPerSec:", 20) << BPB_BytsPerSec << endl;
    os << fixlen("BPB_SecPerClus:", 20) << BPB_SecPerClus << endl;
    os << fixlen("BPB_RsvdSecCnt:", 20) << BPB_RsvdSecCnt << endl;
    os << fixlen("BPB_NumFATs:", 20) << BPB_NumFATs << endl;
    os << fixlen("BPB_RootEntCnt:", 20) << BPB_RootEntCnt << endl;
    os << fixlen("BPB_TotSec16:", 20) << BPB_TotSec16 << endl;
    os << fixlen("BPB_Media:", 20) << format("%02x") % (int)BPB_Media << endl;
    os << fixlen("BPB_FATSz16:", 20) << BPB_FATSz16 << endl;
    os << fixlen("BPB_SecPerTrk:", 20) << BPB_SecPerTrk << endl;
    os << fixlen("BPB_NumHeads:", 20) << BPB_NumHeads << endl;
    os << fixlen("BPB_HiddSec:", 20) << BPB_HiddSec << endl;
    os << fixlen("BPB_TotSec32:", 20) << BPB_TotSec32 << endl;


    if (fat_type == 12 || fat_type == 16)
    {
        os << fixlen("BS_DrvNum:", 20) << fat12_16.BS_DrvNum << endl;
        os << fixlen("BS_Reserved1:", 20) << fat12_16.BS_Reserved1 << endl;
        os << fixlen("BS_BootSig:", 20) << fat12_16.BS_BootSig << endl;
        os << fixlen("BS_VolID:", 20) << format("%08x") % fat12_16.BS_VolID << endl;
        os << fixlen("BS_FilSysType:", 20) << fs_str_12_16 << endl;
    }
    else
    {
        os << fixlen("BPB_FATSz32:", 20) << fat32.BPB_FATSz32 << endl;
        os << fixlen("BPB_ExtFlag:", 20) << fat32.BPB_ExtFlags << endl;
        os << fixlen("BPB_FSVer:", 20)
            << format("%d.%d")
            % (int)fat32.BPB_FSVer[1] % (int)fat32.BPB_FSVer[0]
            << endl;
        os << fixlen("BPB_RootClus:", 20) << fat32.BPB_RootClus << endl;
        os << fixlen("BPB_FSInfo:", 20) << fat32.BPB_FSInfo << endl;
        os << fixlen("BPB_BkBootSec:", 20) << fat32.BPB_BkBootSec << endl;
        os << fixlen("BS_DrvNum:", 20) << fat32.BS_DrvNum << endl;
        os << fixlen("BS_Reserved1:", 20) << fat32.BS_Reserved1 << endl;
        os << fixlen("BS_BootSig:", 20) << fat32.BS_BootSig << endl;
        os << fixlen("BS_VolID:", 20) << format("%08x") % fat32.BS_VolID << endl;
        os << fixlen("BS_FilSysType:", 20) << fs_str_32 << endl;
    }
}

void PbrFat::print_asm(std::ostream& os) const
{
}

