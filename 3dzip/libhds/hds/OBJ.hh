#pragma once
#ifndef HDS_OBJ_HH
#define HDS_OBJ_HH

#include <vector>

namespace OBJ
{
	void load(const char* filename, std::vector<unsigned>& tri, std::vector<float>& geom);
	void store(const char* filename, std::vector<unsigned>& tri, std::vector<float>& geom);
}

#endif
