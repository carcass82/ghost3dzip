#include "utils.h"

namespace Utils
{

	void BuildLaplacian(Eigen::DynamicSparseMatrix<float> &L, const unsigned int *triList, const int triCount)
	{
		for (int i = 0; i < triCount * 3; i += 3) {

			L.coeffRef(triList[i],     triList[i + 1]) = -1.0;
			L.coeffRef(triList[i + 1], triList[i])     = -1.0;

			L.coeffRef(triList[i],     triList[i + 2]) = -1.0;
			L.coeffRef(triList[i + 2], triList[i])     = -1.0;

			L.coeffRef(triList[i + 1], triList[i + 2]) = -1.0;
			L.coeffRef(triList[i + 2], triList[i + 1]) = -1.0;

		}

		for (int i = 0; i < L.rows(); ++i)
			L.coeffRef(i, i) = L.innerNonZeros(i) - 1;
	}

	void UpdateMesh(unsigned int *f, unsigned int fsize, float *v, unsigned int vsize, unsigned int *vorder, unsigned int *forder)
	{
		// step 1: reorder vertices
		// e.g. v[0] (x y z) ---> v[2] (x y z)
		{
			std::vector<float> vReordered(vsize);
			for (int orig = 0; orig < vsize / 3; ++orig)
				memcpy(&vReordered[ vorder[orig] * 3 ], v + (orig * 3), 3 * sizeof(float));
			memcpy(v, &vReordered[0], vsize * sizeof(float));
		}

		// step 2: reorder faces
		// e.g. f[0] (v1 v2 v3) ---> f[2] (v1 v2 v3)
		{
			std::vector<unsigned> fReordered(fsize);
			for (int orig = 0; orig < fsize / 3; ++orig)
				memcpy(&fReordered[ forder[orig] * 3 ], f + (orig * 3), 3 * sizeof(unsigned));
			memcpy(f, &fReordered[0], fsize * sizeof(unsigned));
		}

		// step 3: "rename" vertices inside face definition
		// e.g. f[0] (v1 v2 v3) ---> f[0] (v4 v5 v6)
		for (int faceidx = 0; faceidx < fsize; ++faceidx) {
			f[faceidx] = vorder[f[faceidx]];
		}
	}

	void OrderIndices(unsigned int *first, unsigned int *last)
	{
		// order vertex that define faces in ascending order
		// so that face with v1 v2 and v3 will always be
		// [v1 v2 v3]
		for (; first != last; first += 3)
			std::sort(first, (first + 3));
	}

	double ComputeRMSE(std::vector<float> &origV, std::vector<float> &compV)
	{
//		Eigen::Map<Eigen::MatrixXf> a(&origV[0], 3, origV.size() / 3);
//		Eigen::Map<Eigen::MatrixXf> b(&compV[0], 3, compV.size() / 3);

		Eigen::Map<Eigen::VectorXf> a(&origV[0], origV.size());
		Eigen::Map<Eigen::VectorXf> b(&compV[0], compV.size());

		return sqrtf((a - b).cwise().square().sum()) / a.norm();

//		return sqrt((a.row(0) - b.row(0)).cwise().square().sum() / a.row(0).size()) +
//			   sqrt((a.row(1) - b.row(1)).cwise().square().sum() / a.row(1).size()) +
//			   sqrt((a.row(2) - b.row(2)).cwise().square().sum() / a.row(2).size());
	}

	namespace Timer
	{
#ifndef WIN32
		void Start(instant_t *t1)
		{
			gettimeofday(t1, NULL);
		}

		void Stop(instant_t *t2)
		{
			gettimeofday(t2, NULL);
		}

		double Elapsed(instant_t t1, instant_t t2)
		{
			long secs = t2.tv_sec - t1.tv_sec;
			long usecs = t2.tv_usec - t1.tv_usec;
			return ((double)secs * 1000.0 + (double)usecs * 0.001) + 0.5;
		}
#else
		void Start(instant_t *t1)
		{
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			*t1 = li.QuadPart;
		}

		void Stop(instant_t *t2)
		{
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			*t2 = li.QuadPart;
		}

		double Elapsed(instant_t t1, instant_t t2)
		{
			LARGE_INTEGER li;
			QueryPerformanceFrequency(&li);
			return (t2 - t1) / (li.QuadPart * 0.001);
		}
#endif
	}

}
