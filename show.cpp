#include "show.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <boost/format.hpp>
using namespace std;
using namespace boost;

namespace
{
    class FixedLength
    {
        string s_;
        size_t length_;

        friend ostream& operator <<(ostream& os, FixedLength& f)
        {
            os << f.s_;
            for (size_t i = f.s_.length(); i < f.length_; ++i)
            {
                os.put(' ');
            }
            return os;
        }

    public:
        FixedLength(string s, size_t length)
            : s_(s), length_(length)
        {
        }
    };

    template <typename S, typename L>
    FixedLength fixlen(S&& s, L&& l)
    {
        return FixedLength(std::forward<S>(s), std::forward<L>(l));
    }

    class HexBytes
    {
        basic_string<uint8_t> s_;

        friend ostream& operator <<(ostream& os, HexBytes& b)
        {
            auto pat = format("%02x");
            cout << pat % (int)b.s_[0];
            for (size_t i = 1; i < b.s_.length(); ++i)
            {
                cout << ' ' << pat % (int)b.s_[i];
            }
            return os;
        }

    public:
        template <size_t N>
        HexBytes(const uint8_t (&v)[N])
            : s_(v, N)
        {

        }
    };

    template <typename T>
    HexBytes bytes(T&& v)
    {
        return HexBytes(std::forward<T>(v));
    }

    ostream& operator <<(ostream& os, uint8_t v)
    {
        os << (int)v;
        return os;
    }
}

void show(PbrFat& pbr)
{
    // Determin FAT type
    size_t root_dir_sectors =
        (pbr.BPB_RootEntCnt * 32 + pbr.BPB_BytsPerSec - 1) / pbr.BPB_BytsPerSec;

    const size_t fat_size =
        pbr.BPB_FATSz16 != 0 ? pbr.BPB_FATSz16 : pbr.fat32.BPB_FATSz32;

    const size_t total_sectors =
        pbr.BPB_TotSec16 != 0 ? pbr.BPB_TotSec16 : pbr.BPB_TotSec32;

    const size_t data_sectors = total_sectors
        - (pbr.BPB_RsvdSecCnt + pbr.BPB_NumFATs * fat_size + root_dir_sectors);

    const size_t count_of_clusters = data_sectors / pbr.BPB_SecPerClus;

    const int fat_type =
        count_of_clusters < 4085 ? 12
        : count_of_clusters < 65525 ? 16
        : 32;

    cout << "Calculated fat type is FAT" << fat_type << endl;

    // Check file system string
    string fs_str_12_16(reinterpret_cast<char *>(pbr.fat12_16.BS_FilSysType), 8);
    string fs_str_32(reinterpret_cast<char *>(pbr.fat32.BS_FilSysType), 8);

    if ((fs_str_12_16 == "FAT12   "
        || fs_str_12_16 == "FAT16   "
        || fs_str_12_16 == "FAT     ") && fat_type == 32)
    {
        cout << "But there is a string indicating file system is FAT12 or 16." << endl;
    }

    if (fs_str_32 == "FAT32   " && (fat_type == 12 || fat_type == 16))
    {
        cout << "But there is a string indicating file system is FAT32." << endl;
    }

    cout << fixlen("BS_jmpBoot:", 20) << bytes(pbr.BS_jmpBoot) << endl;
    cout << fixlen("BS_OEMName:", 20)
        << string(reinterpret_cast<char *>(pbr.BS_OEMName), 8) << endl;
    cout << fixlen("BPB_BytsPerSec:", 20) << pbr.BPB_BytsPerSec << endl;
    cout << fixlen("BPB_SecPerClus:", 20) << pbr.BPB_SecPerClus << endl;
    cout << fixlen("BPB_RsvdSecCnt:", 20) << pbr.BPB_RsvdSecCnt << endl;
    cout << fixlen("BPB_NumFATs:", 20) << pbr.BPB_NumFATs << endl;
    cout << fixlen("BPB_RootEntCnt:", 20) << pbr.BPB_RootEntCnt << endl;
    cout << fixlen("BPB_TotSec16:", 20) << pbr.BPB_TotSec16 << endl;
    cout << fixlen("BPB_Media:", 20) << format("%02x") % (int)pbr.BPB_Media << endl;
    cout << fixlen("BPB_FATSz16:", 20) << pbr.BPB_FATSz16 << endl;
    cout << fixlen("BPB_SecPerTrk:", 20) << pbr.BPB_SecPerTrk << endl;
    cout << fixlen("BPB_NumHeads:", 20) << pbr.BPB_NumHeads << endl;
    cout << fixlen("BPB_HiddSec:", 20) << pbr.BPB_HiddSec << endl;
    cout << fixlen("BPB_TotSec32:", 20) << pbr.BPB_TotSec32 << endl;


    if (fat_type == 12 || fat_type == 16)
    {
        cout << fixlen("BS_DrvNum:", 20) << pbr.fat12_16.BS_DrvNum << endl;
        cout << fixlen("BS_Reserved1:", 20) << pbr.fat12_16.BS_Reserved1 << endl;
        cout << fixlen("BS_BootSig:", 20) << pbr.fat12_16.BS_BootSig << endl;
        cout << fixlen("BS_VolID:", 20) << format("%08x") % pbr.fat12_16.BS_VolID << endl;
        cout << fixlen("BS_FileSysType:", 20) << pbr.fat12_16.BS_FilSysType << endl;
    }
    else
    {
        cout << fixlen("BPB_FATSz32:", 20) << pbr.fat32.BPB_FATSz32 << endl;
        cout << fixlen("BPB_ExtFlag:", 20) << pbr.fat32.BPB_ExtFlags << endl;
        cout << fixlen("BPB_FSVer:", 20)
            << format("%d.%d")
            % (int)pbr.fat32.BPB_FSVer[1] % (int)pbr.fat32.BPB_FSVer[0]
            << endl;
        cout << fixlen("BPB_RootClus:", 20) << pbr.fat32.BPB_RootClus << endl;
        cout << fixlen("BPB_FSInfo:", 20) << pbr.fat32.BPB_FSInfo << endl;
        cout << fixlen("BPB_BkBootSec:", 20) << pbr.fat32.BPB_BkBootSec << endl;
        cout << fixlen("BS_DrvNum:", 20) << pbr.fat32.BS_DrvNum << endl;
        cout << fixlen("BS_Reserved1:", 20) << pbr.fat32.BS_Reserved1 << endl;
        cout << fixlen("BS_BootSig:", 20) << pbr.fat32.BS_BootSig << endl;
        cout << fixlen("BS_VolID:", 20) << format("%08x") % pbr.fat32.BS_VolID << endl;
        cout << fixlen("BS_FileSysType:", 20) << pbr.fat32.BS_FilSysType << endl;
    }
}
