#include <string>
#include <vector>
#include <sstream>
#include <fstream>

namespace OBJ
{
	void load(const char* filename, std::vector<unsigned>& tri, std::vector<float>& geom)
	{
		std::ifstream is(filename);

		unsigned int face[3];
		float vertex[3];

		std::string line;
		while (std::getline(is, line)) {
			std::stringstream ss(line);
			std::string token;
			ss >> token;

			if (token == "v") {
				for (int i = 0; i < 3; ++i) {
					ss >> vertex[i];
					geom.push_back(vertex[i]);
				}
			}

			if (token == "f") {
				for (int i = 0; i < 3; ++i) {
					ss >> face[i];
					tri.push_back(face[i]);
				}
			}
		}
		is.close();
	}


	void store(const char* filename, std::vector<unsigned>& tri, std::vector<float>& geom)
	{
		std::ofstream outfile(filename);

		// write geometry
		for (unsigned i = 0; i < geom.size(); i += 3)
			outfile << "v " <<  geom[i] << " " << geom[i + 1] << " " << geom[i + 2] << "\n";

		// write topology
		for (unsigned i = 0; i < tri.size(); i += 3)
			outfile << "f " << tri[i] <<  " " << tri[i + 1] <<  " " << tri[i + 2] << "\n";

		outfile.close();
	}
}
