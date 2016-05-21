#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <Eigen/Eigen>

#ifdef WIN32
	#define NOMINMAX
	#include <windows.h>
#else
	#include <sys/time.h>
#endif

namespace Utils
{
	void BuildLaplacian(Eigen::DynamicSparseMatrix<float> &L, const unsigned int *triList, const int triCount);
	void UpdateMesh(unsigned int *f, unsigned int fsize, float *v, unsigned int vsize, unsigned int *vorder, unsigned int *forder);
	void OrderIndices(unsigned int *first, unsigned int *last);
	double ComputeRMSE(std::vector<float> &origV, std::vector<float> &compV);

	namespace Timer
	{
#ifndef WIN32
		typedef struct timeval instant_t;
#else
		typedef long long instant_t;
#endif
		void Start(instant_t *t1);
		void Stop(instant_t *t2);
		double Elapsed(instant_t t1, instant_t t2);
	};
}

#endif
