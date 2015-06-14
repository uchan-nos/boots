// boots.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>

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
            ("help", "Show this help")
            ("type,t", po::value<string>(), "Type of boot sector. 'mbr' or 'pbr'")
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
        uint8_t buf[512];
        input.read(reinterpret_cast<char *>(buf), 512);
        if (input.gcount() != 512)
        {
            cout << "Size of input file must be 512 bytes." << input.gcount() << endl;
            return 1;
        }

        if (vm.count("type"))
        {
            string type = vm["type"].as<string>();

            PbrFat pbr;
            if (type == "pbr")
            {
                read(pbr, buf);
                show(pbr);
            }
        }
    }
    catch (const po::error& e)
    {
        cout << e.what() << endl;
        return 1;
    }
    return 0;
}

