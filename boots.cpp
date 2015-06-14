// boots.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include <iostream>
#include <array>
#include <fstream>
#include <string>
#include <vector>
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
            ("help", "Show this help")
            ("type,t", po::value<string>(), "Type of boot sector. 'mbr' or 'fat-pbr'")
            ("asm", "Show disassembled code")
            ("input", po::value<string>(), "input file");

        po::positional_options_description pdesc;
        pdesc.add("input", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(pdesc).run(), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            show_usage(cout, desc);
            return 0;
        }
        if (vm.count("input") != 1)
        {
            cerr << "Need an input file\n\n";
            show_usage(cerr, desc);
            return 1;
        }

        ifstream input(vm["input"].as<string>(), ios::in | ios::binary);
        array<uint8_t, 512> bs_buf;
        input.read(reinterpret_cast<char *>(bs_buf.data()), bs_buf.size());
        if (input.gcount() != 512)
        {
            cout << "Size of input file must be 512 bytes." << input.gcount() << endl;
            return 1;
        }

        boost::optional<string> bs_type = vm.count("type") ? vm["type"].as<string>() : predict_type(bs_buf);

        PbrFat pbr;
        if (bs_type)
        {
            if (*bs_type == "fat-pbr")
            {
                read(pbr, bs_buf.data());
                if (vm.count("asm"))
                {
                    show_asm(pbr);
                }
                else
                {
                    show(pbr);
                }
            }
            else
            {
                cerr << "Unknown type: " << *bs_type << endl;
                return 1;
            }
        }
        else
        {
            cerr << "Failed to predict type of boot sector." << endl;
            return 1;
        }
    }
    catch (const po::error& e)
    {
        cout << e.what() << endl;
        return 1;
    }
    return 0;
}

