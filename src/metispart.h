#pragma once
#ifndef METISPART_H
#define METISPART_H

#include <iostream>
#include <vector>
#include <map>
#include <tr1/unordered_map>

#include <Eigen/Sparse>

extern "C" {
  #include <metis.h>
}

#ifdef USE_OPENMP
 #include <omp.h>
#endif

#include "hds/HDS.hh"
#include "metispartition.h"

namespace Metis
{
	std::vector<MetisPartition>
	Split(unsigned int *faces, const int N_FACES, const int N_VERT, const int SPLIT);

	void BuildLaplacian(Eigen::MatrixXf &L, const MetisPartition &p, const unsigned int *triList);
};

#endif // METISPART_H
