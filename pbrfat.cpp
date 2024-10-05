#include "bootsector.hpp"

// for numeric_limits::max
#define NOMINMAX

#include <vector>
#include <string>
#include <utility>
#include <limits>
#include <sstream>
#include <type_traits>
#include <fstream>
#include <Poco/Process.h>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include "read.hpp"
#include "show.hpp"
#include "tempfile.hpp"

using namespace std;
using namespace boost;
using namespace readutil;
using namespace showutil;

namespace
{
    const char dbwd_table[4] = {'b', 'w', 0, 'd'};

    template <typename T>
    uint32_t dbwd(ostream& os, uint32_t current_address, uint32_t limit_address,
        T value, typename std::enable_if<sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4>::type* = 0)
    {
        if (current_address + sizeof(T) < limit_address)
        {
            os << "    d" << dbwd_table[sizeof(T) - 1] << " " << value << '\n';
            return current_address + sizeof(T);
        }
        return current_address;
    }

    uint32_t db_str(ostream& os, uint32_t current_address, uint32_t limit_address,
        const uint8_t* value, size_t length)
    {
        if (current_address + length < limit_address)
        {
            os << "    db \"" << string(reinterpret_cast<const char*>(value), length) << "\"\n";
            return current_address + length;
        }
        return current_address;
    }
}

PbrFat::PbrFat(const std::array<uint8_t, 512>& data)
    : BootSector(data)
{
    const uint8_t* data_ = data.data();

    data_ = read(this->BS_jmpBoot, data_);
    data_ = read(this->BS_OEMName, data_);
    data_ = read16(this->BPB_BytsPerSec, data_);
    data_ = read8(this->BPB_SecPerClus, data_);
    data_ = read16(this->BPB_RsvdSecCnt, data_);
    data_ = read8(this->BPB_NumFATs, data_);
    data_ = read16(this->BPB_RootEntCnt, data_);
    data_ = read16(this->BPB_TotSec16, data_);
    data_ = read8(this->BPB_Media, data_);
    data_ = read16(this->BPB_FATSz16, data_);
    data_ = read16(this->BPB_SecPerTrk, data_);
    data_ = read16(this->BPB_NumHeads, data_);
    data_ = read32(this->BPB_HiddSec, data_);
    data_ = read32(this->BPB_TotSec32, data_);

    const uint8_t* data0 = data_;

    data_ = read8(this->fat12_16.BS_DrvNum, data_);
    data_ = read8(this->fat12_16.BS_Reserved1, data_);
    data_ = read8(this->fat12_16.BS_BootSig, data_);
    data_ = read32(this->fat12_16.BS_VolID, data_);
    data_ = read(this->fat12_16.BS_VolLab, data_);
    data_ = read(this->fat12_16.BS_FilSysType, data_);
    data_ = read(this->fat12_16.boot_program, data_);

    data_ = data0;

    data_ = read32(this->fat32.BPB_FATSz32, data_);
    data_ = read16(this->fat32.BPB_ExtFlags, data_);
    data_ = read(this->fat32.BPB_FSVer, data_);
    data_ = read32(this->fat32.BPB_RootClus, data_);
    data_ = read16(this->fat32.BPB_FSInfo, data_);
    data_ = read16(this->fat32.BPB_BkBootSec, data_);
    data_ = read(this->fat32.BPB_Reserved, data_);
    data_ = read8(this->fat32.BS_DrvNum, data_);
    data_ = read8(this->fat32.BS_Reserved1, data_);
    data_ = read8(this->fat32.BS_BootSig, data_);
    data_ = read32(this->fat32.BS_VolID, data_);
    data_ = read(this->fat32.BS_VolLab, data_);
    data_ = read(this->fat32.BS_FilSysType, data_);
    data_ = read(this->fat32.boot_program, data_);

    data_ = read(this->last_signature, data_);
}

void PbrFat::print_info(std::ostream& os) const
{
    const int fat_type = determine_fat_type();

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
        os << fixlen("BS_VolLab:", 20)
            << string(reinterpret_cast<const char *>(fat12_16.BS_VolLab), 11) << endl;
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
        os << fixlen("BS_VolLab:", 20)
            << string(reinterpret_cast<const char *>(fat32.BS_VolLab), 11) << endl;
        os << fixlen("BS_FilSysType:", 20) << fs_str_32 << endl;
    }
}

void PbrFat::print_asm(std::ostream& os) const
{
    const int fat_type = determine_fat_type();

    unsigned int skipStart = 0, skipBytes = 0;
    if (BS_jmpBoot[0] == 0xeb)
    {
        // short jump
        skipBytes = BS_jmpBoot[1] - 1;
    }
    else if (BS_jmpBoot[0] == 0xe9)
    {
        // near jump
        skipBytes = BS_jmpBoot[1]
            | (static_cast<unsigned int>(BS_jmpBoot[2]) << 8);
    }

    // write bs to temporary file.
    // it will be removed at the end of this function automatically by TemporaryFile's destructor.
    TemporaryFile temp_file;
    ofstream temp_ofs(temp_file.path().native(), ios::out | ios::binary);
    const auto& bs_data = this->data();
    temp_ofs.write(reinterpret_cast<const char*>(bs_data.data()), bs_data.size());
    temp_ofs.flush();
    temp_ofs.close();

    Poco::Process::Args args;
    args.push_back("-b");
    args.push_back("16");
    args.push_back("-k");
    args.push_back(str(format("3,%d") % skipBytes)); // skip bpb section
    args.push_back("-k");
    args.push_back("510,2"); // skip last signature
    args.push_back(temp_file.path().string());

    Poco::Pipe stdout_pipe;
    auto proc = Poco::Process::launch("ndisasm", args, nullptr, &stdout_pipe, nullptr);
    Poco::PipeInputStream is(stdout_pipe);

    vector<pair<uint32_t, string>> instructions;
    int skip_index = -1; // index of "skipping" line.
    uint32_t zero_region_address = numeric_limits<uint32_t>::max(); // start address of the last all zero region.

    const regex disasm_pattern("^([0-9A-Fa-f]+)\\s+([0-9A-Fa-f]+)\\s+(.+)$");
    const regex skip_pattern("^[0-9A-Fa-f]+\\s+skipping.*$");

    string line;
    while (getline(is, line))
    {
        smatch m;
        if (regex_match(line, m, disasm_pattern))
        {
            // convert hex string to integer
            uint32_t address;
            stringstream ss;
            ss << hex << m[1];
            ss >> address;

            if (m[2] == "0000")
            {
                if (zero_region_address == numeric_limits<uint32_t>::max())
                {
                    zero_region_address = address;
                }
            }
            else
            {
                zero_region_address = numeric_limits<uint32_t>::max();
                instructions.push_back(make_pair(address, m[3]));
            }
        }
        else if (regex_match(line, m, skip_pattern))
        {
            if (skip_index == -1)
            {
                skip_index = instructions.size();
            }
        }
    }

    for (int i = 0; i < instructions.size(); ++i)
    {
        if (i == skip_index)
        {
            uint32_t current = i > 0 ? instructions[i - 1].first : 0;
            const uint32_t lim = instructions[i].first;

            current = db_str(os, current, lim, BS_OEMName, 8);
            current = dbwd(os, current, lim, BPB_BytsPerSec);
            current = dbwd(os, current, lim, BPB_SecPerClus);
            current = dbwd(os, current, lim, BPB_RsvdSecCnt);
            current = dbwd(os, current, lim, BPB_NumFATs);
            current = dbwd(os, current, lim, BPB_RootEntCnt);
            current = dbwd(os, current, lim, BPB_TotSec16);
            current = dbwd(os, current, lim, BPB_Media);
            current = dbwd(os, current, lim, BPB_FATSz16);
            current = dbwd(os, current, lim, BPB_SecPerTrk);
            current = dbwd(os, current, lim, BPB_NumHeads);
            current = dbwd(os, current, lim, BPB_HiddSec);
            current = dbwd(os, current, lim, BPB_TotSec32);

            if (fat_type == 12 || fat_type == 16)
            {
                current = dbwd(os, current, lim, fat12_16.BS_DrvNum);
                current = dbwd(os, current, lim, fat12_16.BS_Reserved1);
                current = dbwd(os, current, lim, fat12_16.BS_BootSig);
                current = dbwd(os, current, lim, fat12_16.BS_VolID);
                current = db_str(os, current, lim, fat12_16.BS_VolLab, 11);
                current = db_str(os, current, lim, fat12_16.BS_FilSysType, 8);
            }
            else
            {
                current = dbwd(os, current, lim, fat32.BPB_FATSz32);
                current = dbwd(os, current, lim, fat32.BPB_ExtFlags);
                current = dbwd(os, current, lim, fat32.BPB_FSVer);
                current = dbwd(os, current, lim, fat32.BPB_RootClus);
                current = dbwd(os, current, lim, fat32.BPB_FSInfo);
                current = dbwd(os, current, lim, fat32.BPB_BkBootSec);
                current = dbwd(os, current, lim, fat32.BPB_Reserved);
                current = dbwd(os, current, lim, fat32.BS_DrvNum);
                current = dbwd(os, current, lim, fat32.BS_Reserved1);
                current = dbwd(os, current, lim, fat32.BS_BootSig);
                current = dbwd(os, current, lim, fat32.BS_VolID);
                current = db_str(os, current, lim, fat32.BS_VolLab, 11);
                current = db_str(os, current, lim, fat32.BS_FilSysType, 8);
            }

            if (current < lim)
            {
                os << "    times " << format("0x%02x") % lim << " - ($ - $$) db 0" << endl;
            }

            os << "entry:" << endl;
        }

        if (i == 0)
        {
            if (BS_jmpBoot[0] == 0xeb)
            {
                os << "    jmp short entry" << endl;
            }
            else
            {
                os << "    jmp near entry" << endl;
            }
        }
        else
        {
            os << "    " << instructions[i].second << endl;
        }
    }

    if (zero_region_address != numeric_limits<uint32_t>::max())
    {
        os << "    times 0x1fe - ($ - $$) db 0" << endl;
    }

    os << "    db 0x55, 0xaa" << endl;
}

int PbrFat::determine_fat_type() const
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

    return fat_type;
}

