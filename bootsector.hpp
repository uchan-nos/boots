#ifndef BOOTSECTOR_HPP_
#define BOOTSECTOR_HPP_

#include <array>
#include <ostream>
#include <memory>
#include <cstdint>

class BootSector
{
    std::array<uint8_t, 512> data_;

public:
    enum class Type
    {
        kMbr, kPbrFat, kUnknown
    };

    BootSector(const std::array<uint8_t, 512>& data);
    const std::array<uint8_t, 512>& data() const;

    virtual ~BootSector();
    virtual void print_info(std::ostream& os) const = 0;
    virtual void print_asm(std::ostream& os) const = 0;
};

class Mbr : public BootSector
{
    uint8_t boot_code[446];

    struct Partition
    {
        uint8_t active;
        uint8_t chs_start[3];
        uint8_t type;
        uint8_t chs_end[3];
        uint32_t lba_start;
        uint32_t size;
    } part_table[4];

    uint8_t last_signature[2];
public:
    Mbr(const std::array<uint8_t, 512>& data);
    void print_info(std::ostream& os) const;
    void print_asm(std::ostream& os) const;
};

class PbrFat : public BootSector
{
    uint8_t BS_jmpBoot[3];
    uint8_t BS_OEMName[8];
    uint16_t BPB_BytsPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;

    struct Fat12_16
    {
        uint8_t BS_DrvNum;
        uint8_t BS_Reserved1;
        uint8_t BS_BootSig;
        uint32_t BS_VolID;
        uint8_t BS_VolLab[11];
        uint8_t BS_FilSysType[8];
        uint8_t boot_program[448];
    } fat12_16;

    struct Fat32
    {
        uint32_t BPB_FATSz32;
        uint16_t BPB_ExtFlags;
        uint8_t BPB_FSVer[2];
        uint32_t BPB_RootClus;
        uint16_t BPB_FSInfo;
        uint16_t BPB_BkBootSec;
        uint8_t BPB_Reserved[12];
        uint8_t BS_DrvNum;
        uint8_t BS_Reserved1;
        uint8_t BS_BootSig;
        uint32_t BS_VolID;
        uint8_t BS_VolLab[11];
        uint8_t BS_FilSysType[8];
        uint8_t boot_program[420];
    } fat32;

    uint8_t last_signature[2];
public:
    PbrFat(const std::array<uint8_t, 512>& data);
    void print_info(std::ostream& os) const;
    void print_asm(std::ostream& os) const;

private:
    int determine_fat_type() const;
};

BootSector::Type infer(const std::array<uint8_t, 512>& data);
std::unique_ptr<BootSector> make_bs(
        const std::array<uint8_t, 512>& data, BootSector::Type type);

unsigned int cylinder(const uint8_t (&chs)[3]);
unsigned int head(const uint8_t (&chs)[3]);
unsigned int sector(const uint8_t (&chs)[3]);

#endif
