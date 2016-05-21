#include "metispart.h"

namespace Metis
{

	std::vector<MetisPartition>
	Split(unsigned int *faces, const int TRIs, const int VERTs, const int SPLIT)
	{
		enum {TRIANGLES = 1, TETRAHEDRA, HEXAHEDRA, QUADRILATERALS};

		int elemcount = TRIs;
		int nodecount = VERTs;
		int type = TRIANGLES;
		idxtype *elems = (idxtype*)faces;   // WARNING: unsigned* to int*
		int startElem = 0;
/*#ifdef USE_OPENMP
		int partCount = omp_get_max_threads();
#else*/
		int partCount = VERTs / SPLIT;
//#endif
		int edgeCut;
		int *elist = new int[elemcount];
		int *nlist = new int[nodecount];

		METIS_PartMeshDual(&elemcount,
						   &nodecount,
						   elems,
						   &type,
						   &startElem,
						   &partCount,
						   &edgeCut,
						   elist,
						   nlist);
/*
#ifdef USE_OPENMP

		std::vector<MetisPartition> partList;

		#pragma omp parallel for
		for (int m = 0; m < partCount; ++m) {

			std::vector<int> pFaces;
			std::vector<int> p2o_tri;
			for (int i = 0; i < TRIs; ++i) {

				if (elist[i] == m) {
					if (nlist[faces[3 * i + 0]] == m &&
						nlist[faces[3 * i + 1]] == m &&
						nlist[faces[3 * i + 2]] == m) {

						pFaces.push_back(faces[3 * i + 0]);
						pFaces.push_back(faces[3 * i + 1]);
						pFaces.push_back(faces[3 * i + 2]);

						p2o_tri.push_back(i);
					}
				}
			}

			std::vector<int> p2o;
			std::tr1::unordered_map<int, int> o2p;
			int count = 0;

			for (int i = 0; i < pFaces.size(); ++i) {

				if (o2p.find(pFaces[i]) == o2p.end()) {
					p2o.push_back(pFaces[i]);
					o2p[pFaces[i]] = count;
					++count;
				}

				pFaces[i] = o2p[pFaces[i]];
			}

			int pPartCount = p2o.size() / SPLIT;
			int pElemCount = pFaces.size() / 3;
			int pNodeCount = p2o.size();

			int *pElist = new int[pElemCount];
			int *pNlist = new int[pNodeCount];

			METIS_PartMeshDual(&pElemCount,
							   &pNodeCount,
							   &pFaces[0],
							   &type,
							   &startElem,
							   &pPartCount,
							   &edgeCut,
							   pElist,
							   pNlist);

			std::vector<MetisPartition> pPartList(pPartCount);
			for (unsigned i = 0; i < pNodeCount; ++i)
				pPartList[nlist[i]].AddVertex(p2o[i]);

			for (unsigned i = 0; i < pElemCount; ++i)
				pPartList[elist[i]].AddTriangle(p2o_tri[i]);

			delete[] pElist;
			delete[] pNlist;

			#pragma omp critical
			std::copy(pPartList.begin(), pPartList.end(), std::back_inserter(partList));
		}

#else
*/

		std::vector<MetisPartition> partList(partCount);
		for (unsigned i = 0; i < nodecount; ++i)
			partList[nlist[i]].AddVertex(i);

		for (unsigned i = 0; i < elemcount; ++i)
			partList[elist[i]].AddTriangle(i);

//#endif
		delete[] elist;
		delete[] nlist;

		return partList;
	}


	void BuildLaplacian(Eigen::MatrixXf &L, const MetisPartition &p, const unsigned int *triList)
	{
		L.setZero();

		for (int i = 0; i < p.TriangleCount(); ++i) {

			unsigned f_o = p.GetTriangleOriginalIndex(i);

			int v_i = p.GetVertexPartitionIndex(triList[f_o * 3]);
			int v_j = p.GetVertexPartitionIndex(triList[f_o * 3 + 1]);
			int v_k = p.GetVertexPartitionIndex(triList[f_o * 3 + 2]);

			if (v_i != -1) {
				if (v_j != -1) {
					L(v_i, v_j) = -1.0;
					L(v_j, v_i) = -1.0;
				}

				if (v_k != -1) {
					L(v_i, v_k) = -1.0;
					L(v_k, v_i) = -1.0;
				}
			}

			if (v_j != -1 && v_k != -1) {
				L(v_k, v_j) = -1.0;
				L(v_j, v_k) = -1.0;
			}
		}

		for (int i = 0; i < p.VertexCount(); ++i)
			L(i, i) = L.row(i).sum() * -1;
	}

}
