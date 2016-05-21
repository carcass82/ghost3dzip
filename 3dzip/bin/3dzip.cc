#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "hds/OFF.hh"
#include "3dzip/Writer.hh"

int main(int argc, char *argv[])
{
    std::string output_filename;
    while (char c = getopt(argc, argv, "o:b:") != -1) {
	switch (c) {
	    case 'o':
		output_filename = optarg;
		break;
	    case 'b':
		break;
	}
    }
    if (argc < optind + 1) {
	std::cerr << "You must supply the name of the file to compress" << std::endl;
	std::exit(EXIT_FAILURE);
    }
    if (output_filename == "")
	output_filename = argv[optind] + std::string(".3dz");
    std::cout << output_filename << std::endl;
    std::ofstream of(output_filename.c_str(), std::ios::binary);
    std::vector<unsigned> tri;
    std::vector<float> ver;
    OFF::load(argv[1], tri, ver);
    VBE::Writer write(&tri[0], tri.size() / 3, ver.size() / 3);
    write.addAttrib(&ver[0], ver.size() / 3, 3, "V");
    write(of);
}
