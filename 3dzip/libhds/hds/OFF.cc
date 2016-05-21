#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

namespace OFF {

void load(const char* filename, std::vector<unsigned>& tri, std::vector<float>& geom)
{
    std::ifstream is(filename);
    std::string off;
    is >> off;
    if (off != "OFF")
	throw std::logic_error("Bad file format");

    unsigned num_ver, num_tri, num_edg;
    is >> num_ver >> num_tri >> num_edg;

    // Read Geometry
    geom.resize(3 * num_ver);
    for (unsigned i = 0; i < 3 * num_ver; i++)
	is >> geom[i];

    // Read Connectivity
    tri.reserve(num_tri);
    for (size_t i = 0; i < num_tri; i++) {
	int num_faces;
	is >> num_faces;
	if (num_faces != 3)
	    throw std::logic_error("Only triangular meshes are accepted");
	std::vector<unsigned> v(3);
	for (int k = 0; k < 3; k++)
	    is >> v[k];
	is.ignore(1024, '\n');
	if (v[0] == v[1] || v[1] == v[2] || v[2] == v[0])
	    continue;
	std::copy(v.begin(), v.end(), std::back_inserter(tri));
    }    
    is.close();
}

void store(const char* filename, std::vector<unsigned>& tri, std::vector<float>& geom)
{
	std::ofstream outfile(filename);

	outfile << "OFF\n";
	outfile << geom.size() / 3 << " " << tri.size() / 3 << " " << 0 << "\n";

	// write geometry
	for (unsigned i = 0; i < geom.size(); i += 3)
		outfile << geom[i] << " " << geom[i + 1] << " " << geom[i + 2] << "\n";

	// write topology
	for (unsigned i = 0; i < tri.size(); i += 3)
		outfile << 3 <<  " " << tri[i] <<  " " << tri[i + 1] <<  " " << tri[i + 2] << "\n";

	outfile.close();
}

}
