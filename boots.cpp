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

int main(int argc, char** argv)
{
    try
    {
        po::options_description desc("Options");
        desc.add_options()
            ("help,h", "Show this help")
            ("type,t", po::value<string>(), "Type of boot sector. 'mbr' or 'pbr-fat'")
            ("asm,a", "Show assembly code")
            ("input", po::value<string>(), "")
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

        BootSector::Type bs_type = BootSector::Type::kUnknown;

        if (vm.count("type"))
        {
            string t = vm["type"].as<string>();

            if (t == "pbr")
            {
                bs_type = BootSector::Type::kPbrFat;
            }
        }
        else
        {
            bs_type = infer(bs_buf);
        }


        if (bs_type == BootSector::Type::kUnknown)
        {
            cout << "Unkonwn boot sector type." << endl;
            return 1;
        }

        unique_ptr<BootSector> bs = make_bs(bs_buf, bs_type);

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

