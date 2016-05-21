#ifndef HDS_OFF_HH
#define HDS_OFF_HH

#include <vector>

namespace OFF {

void load(const char* filename, std::vector<unsigned>& tri, std::vector<float>& geom);
void store(const char* filename, std::vector<unsigned>& tri, std::vector<float>& geom);

}

#endif
