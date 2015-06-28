// boots.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include <iostream>
#include <array>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <boost/program_options.hpp>
#include <boost/optional.hpp>

#include "read.hpp"
#include "show.hpp"
#include "show_asm.hpp"

using namespace std;
namespace po = boost::program_options;

void show_usage(ostream& os, const po::options_description& desc)
{
    os <<
        "Boot sector analyzer\n"
        "Usage: boots [--type TYPE] FILE\n"
        "\n";
    os << desc;
}

boost::optional<string> predict_type(const array<uint8_t, 512>& bs_buf)
{
    if (bs_buf[510] != 0x55 || bs_buf[511] != 0xaa)
    {
        // not bootable
        return boost::optional<string>();
    }
    else if ((bs_buf[0] == 0xeb && bs_buf[2] == 0x90) || bs_buf[0] == 0xe9)
    {
        // right jump instruction
        return boost::optional<string>("fat-pbr");
    }
    else
    {
        // count active flags
        int count00 = 0, count80 = 0;
        for (int i = 0; i < 4; ++i)
        {
            const uint8_t active_flag = bs_buf[0x1be + 16 * i];
            if (active_flag == 0x80)
            {
                ++count80;
            }
            else if (active_flag == 0x00)
            {
                ++count00;
            }
        }

        if ((count00 + count80) == 4)
        {
            // all active flags are 0x00 or 0x80
            return boost::optional<string>("mbr");
        }
    }

    // unknown type
    return boost::optional<string>();
}

int main(int argc, char** argv)
{
    try
    {
        po::options_description desc("Options");
        desc.add_options()
            ("help,h", "Show this help")
            ("type,t", po::value<string>(), "Type of boot sector. 'mbr' or 'pbr-fat'")
            ("asm,a", "Show assembly code")
            ;

        po::positional_options_description pdesc;
        pdesc.add("input", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(pdesc).run(), vm);
        po::notify(vm);

        if (vm.count("help") || vm.count("input") == 0)
        {
            show_usage(cout, desc);
            return 0;
        }

        ifstream input(vm["input"].as<string>(), ios::in | ios::binary);
        array<uint8_t, 512> bs_buf;
        input.read(reinterpret_cast<char *>(bs_buf.data()), bs_buf.size());
        if (input.gcount() != 512)
        {
            cout << "Size of input file must be 512 bytes." << input.gcount() << endl;
            return 1;
        }

        BootSector::Type type = BootSector::Type::kUnknown;

        if (vm.count("type"))
        {
            string t = vm["type"].as<string>();

            if (t == "pbr")
            {
                type = BootSector::Type::kPbrFat;
            }
        }
        else
        {
            type = infer(buf);
        }


        if (type == BootSector::Type::kUnknown)
        {
            cout << "Unkonwn boot sector type." << endl;
            return 1;
        }

        unique_ptr<BootSector> bs = make_bs(buf, type);

        if (!bs)
        {
            cout << "bs is null. bug?" << endl;
            return 1;
        }

        if (vm.count("asm"))
        {
            bs->print_asm(cout);
        }
        else
        {
            bs->print_info(cout);
        }
    }
    catch (const po::error& e)
    {
        cout << e.what() << endl;
        return 1;
    }
    return 0;
}

