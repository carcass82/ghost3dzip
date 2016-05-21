#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

#include "hds/OFF.hh"

#include "3dzip/Writer.hh"
#include "3dzip/Reader.hh"

#include "encoder.h"
#include "decoder.h"



#define ENCODER_PROGNAME "specencode"
#define DECODER_PROGNAME "specdecode"

void usage(std::ostream &os, bool isEncoder)
{
	if (isEncoder) {
		os << "Usage: " << ENCODER_PROGNAME << " [options] <mesh.off>\n"
		   << "OPTIONS:\n"
		   << "  -h, --help              write this help\n"
		   << "  -c, --compression <n>   use only first <n>% coefficients\n"
		   << "  -s, --stats             output statistics to file out.stats\n"
#ifdef HAVE_METIS_H
		   << "  -p, --patchsize <n>     try to split mesh in patches of <n> vertices\n"
#endif
		   << "  -q, --quantization <n>  quantize coefficients with <n> bits\n"
		   << "  -o, --output <file>     save encoded mesh to <file> (defaults to standard output)\n"
		   << std::endl;
	} else {
		os << "Usage: " << DECODER_PROGNAME << " [options] <mesh.off>\n"
		   << "OPTIONS:\n"
		   << "  -h, --help              write this help\n"
		   << "  -i, --input <file>      read compressed mesh from <file> (default: read from standard input)\n"
		   << std::endl;
	}
}

int main(int argc, char **argv)
{
	std::string progName(argv[0]);
	progName = progName.substr(progName.length() - 10);
	argc--; argv++;

	bool isEncodingMode;
	if (progName == ENCODER_PROGNAME) {
		isEncodingMode = true;
	} else if (progName == DECODER_PROGNAME) {
		isEncodingMode = false;
	} else {
		std::cerr << "Please call me as " << ENCODER_PROGNAME << " or " << DECODER_PROGNAME << std::endl;
		return -1;
	}


	if (argc < 1) {
		usage(std::cerr, isEncodingMode);
		return -1;
	}

	bool readFromFile = false;
	std::ifstream inFile;
	bool saveToFile = false;
	std::ofstream outFile;
	bool generateStats = false;
	std::ofstream statsFile;
	size_t compsize, origsize;
	float coeffsRatio = 100;
	std::string inputMesh;
	std::string outputMesh;
	bool useMetis = false;
	int patchsize = 0;
	unsigned char quant = 16;

	for (; argc > 0; argc--, argv++) {
		std::string opt(argv[0]);
		if (opt == "-h" || opt == "--help") {
			usage(std::cerr, isEncodingMode);
			return -1;
		} else if (isEncodingMode) {

			if (opt == "-c" || opt == "--compression") {
				argc--; argv++;
				coeffsRatio = atoi(argv[0]);
			} else if (opt == "-q" || opt == "--quantization") {
				argc--; argv++;
				quant = atoi(argv[0]);
			} else if (opt == "-s" || opt == "--stats") {
				generateStats = true;
				statsFile.open("out.stats");
			} else if (opt == "-o" || opt == "--output") {
				argc--; argv++;
				saveToFile = true;
				outFile.open(argv[0], std::ios::binary);
			} else if (opt == "-p" || opt == "--patchsize") {
#ifdef HAVE_METIS_H
				argc--; argv++;
				patchsize = atoi(argv[0]);
				useMetis = true;
#else
				std::cerr << "Split requested but MeTiS support not compiled in!" << std::endl;
				return -1;
#endif
			} else {
				inputMesh = argv[0];
			}

		} else if (!isEncodingMode) {

			if (opt == "-i" || opt == "--input") {
				argc--; argv++;
				readFromFile = true;
				inFile.open(argv[0], std::ios::binary);
			} else {
				outputMesh = argv[0];
			}

		} else {

			usage(std::cerr, isEncodingMode);
			return -1;

		}
	}


	if (coeffsRatio < 1 || coeffsRatio > 100) {
		std::cerr << "Compression ratio must be an integer between 1 and 100" << std::endl;
		return -1;
	}

	if (isEncodingMode && inputMesh.empty()) {
		std::cerr << "Please specify input mesh (see " << ENCODER_PROGNAME << " -h)" << std::endl;
		return -1;
	}

	if (!isEncodingMode && outputMesh.empty()) {
		std::cerr << "Please specify output mesh (see " << DECODER_PROGNAME << " -h)" << std::endl;
		return -1;
	}


	if (isEncodingMode) {

		std::ostringstream oss(std::ios::binary);

		std::ostringstream oss_stats;
		Utils::Timer::instant_t start, stop;

		if (generateStats)
			Utils::Timer::Start(&start);

		/* load mesh with OFF loader */
		std::vector<unsigned int> faces;
		std::vector<float> vertices;
		OFF::load(inputMesh.c_str(), faces, vertices);

		if (generateStats) {
			Utils::Timer::Stop(&stop);

			oss_stats << "Mesh \"" <<  inputMesh << "\""
					  << " (" << vertices.size() / 3 << " V / " << faces.size() / 3 << " F)"
					  << " loaded in " << Utils::Timer::Elapsed(start, stop) << " msecs\n" << std::endl;

			Utils::Timer::Start(&start);
		}

		/* compress topology */
		VBE::Writer write(&faces[0], faces.size() / 3, vertices.size() / 3, true);
		unsigned *vo = write.getVertexOrdering();
		unsigned *fo = write.getFaceOrdering();

		write(oss);

		if (generateStats) {
			Utils::Timer::Stop(&stop);

			compsize = oss.str().size();
			origsize = faces.size() * sizeof(unsigned int);

			oss_stats << "Mesh topology compressed in " << Utils::Timer::Elapsed(start, stop) << " msecs"
					  << " using " << double(compsize) * CHAR_BIT / (vertices.size() / 3.0) << " bits per vertex\n"
					  << "compressed size: " << compsize << " bytes\n"
					  << "uncompressed size: " << origsize << " bytes\n"
					  << "(compression ratio: " << (double)compsize / origsize << ")\n" << std::endl;
		}

		// update vertex and face ordering based on topology compression
		Utils::UpdateMesh(&faces[0], faces.size(), &vertices[0], vertices.size(), vo, fo);

		delete[] vo;
		delete[] fo;

		// this step is only necessary when splitting mesh
		if (useMetis)
			Utils::OrderIndices(&faces[0], &faces[0] + faces.size());

		/* compress geometry */
		Encoder enc(&vertices[0], &faces[0], vertices.size(), faces.size());

		if (generateStats)
			Utils::Timer::Start(&start);

		enc.Encode(oss, coeffsRatio, patchsize, quant);

		if (generateStats) {
			Utils::Timer::Stop(&stop);

			compsize = oss.str().size() - compsize;
			origsize = vertices.size() * sizeof(float);

			oss_stats << "Mesh geometry compressed in " << Utils::Timer::Elapsed(start, stop) << " msecs"
					  << " using " << double(compsize) * CHAR_BIT / (vertices.size() / 3.0) << " bits per vertex\n"
					  << "compressed size: " << compsize << " bytes\n"
					  << "uncompressed size: " << origsize << " bytes\n"
					  << "(compression ratio: " << (double)compsize / origsize << ")\n" << std::endl;

			compsize = oss.str().size();
			origsize = faces.size() * sizeof(unsigned int) + vertices.size() * sizeof(float);

			oss_stats << "File compressed in " << Utils::Timer::Elapsed(start, stop) << " msecs"
					  << " using " << double(compsize) * CHAR_BIT / (vertices.size() / 3.0) << " bits per vertex\n"
					  << "compressed size: " << compsize << " bytes\n"
					  << "uncompressed size: " << origsize << " bytes\n"
					  << "(compression ratio: " << (double)compsize / origsize << ")\n" << std::endl;
		}

		oss.flush();
		if (saveToFile) {
			outFile << oss.str();
			outFile.close();
		} else {
			std::cout << oss.str();
		}

		if (generateStats) {
			oss_stats.flush();
			statsFile << oss_stats.str();
			statsFile.close();
		}

	} else {  // !isEncodingMode

		/* read back topology */
		VBE::Reader read;
		if (readFromFile)
			read(inFile);
		else
			read(std::cin);

		unsigned *f = read.getTriangles();
		unsigned fsize = read.getNumTri() * 3;

		Utils::OrderIndices(f, f + fsize);

		/* read back geometry */
		Decoder dec(f, fsize, read.getNumVer());

		if (readFromFile)
			dec.Decode(inFile);
		else
			dec.Decode(std::cin);

		if (readFromFile)
			inFile.close();

		std::vector<unsigned> faces(f, f + fsize);
		OFF::store(outputMesh.c_str(), faces, dec.getVertices());
	}
}
