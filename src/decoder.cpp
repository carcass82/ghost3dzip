#include "decoder.h"

Decoder::Decoder(unsigned int *f, unsigned int fsize, const int nVert)
{
	m_VertCount = nVert;
	m_FaceCount = fsize / 3;

	m_Faces = f;
	m_Vertices.resize(m_VertCount * 3);
}

void Decoder::Decode(std::istream &is)
{
	// read total coefficients saved
	int nCoeffs;
	is.read((char*)&nCoeffs, sizeof(int));

	// read if mesh is splitted
	unsigned short splitThreshold;
	is.read((char*)&splitThreshold, sizeof(unsigned short));

	unsigned char q;
	is.read((char*)&q, sizeof(unsigned char));

	if (splitThreshold == 0) {

		Eigen::DynamicSparseMatrix<float> L(m_VertCount, m_VertCount);
		Utils::BuildLaplacian(L, m_Faces, m_FaceCount);
		EigenSolver s(L);

		std::vector<float> coefficients(nCoeffs);
		for (int offset = 0; offset < 3; ++offset) {

			std::vector<int> vQuantCoefficients(nCoeffs);

#ifdef USE_HUFFMAN_CODER
			// huffman-decode coefficients
			int hSize;
			is.read((char*)&hSize, sizeof(int));
			std::vector<unsigned char> hCoeffs(hSize);
			is.read((char*)&hCoeffs[0], hSize * sizeof(unsigned char));

			Huffman_Uncompress((unsigned char*)&hCoeffs[0],
							   (unsigned char*)&vQuantCoefficients[0],
							   hSize,
							   nCoeffs * sizeof(int));
#else
			RangeDec r(is);
			VBE::action::in::Range<RangeDec, MoffatModel> aritdecoder(1 << ((q == 0)? (16 + 1) : q), &r);
			r.start();
			for (int i = 0; i < nCoeffs; ++i)
				aritdecoder(vQuantCoefficients[i]);
			r.done();
#endif

			if (q != 0) { // uniform quantization

				BoundingBox<float> bb(1);
				bb.unserialize(is);

				QVertexAttrib<> qCoeffs(q, bb, nCoeffs, 1);
				qCoeffs.setRow() = Eigen::Map<Eigen::VectorXi>(&vQuantCoefficients[0], nCoeffs);
				qCoeffs.dequantize(&coefficients[0]);

			} else {

				// non uniform quantization
				// | 5% | 10% | 10% | 75% |   ( % spectral coefficients )
				// +----+-----+-----+-----+
				// | 12 | 10  | 8   |  4  |   ( q )
				const int coeffVals[] = { 5, 10, 10, 75 };
				const int qVals[] = { 12, 10,  8,  4 };

				for (int i = 0; i < 4; ++i) {

					int nCoeffsVarStart = 0;
					if (i != 0) {
						for (int j = 0; j < i; ++j)
							nCoeffsVarStart += coeffVals[j];
						nCoeffsVarStart *= (float)nCoeffs * 0.01;
					}

					int nCoeffsVar = (float)nCoeffs * 0.01 * coeffVals[i];
					int _q = qVals[i];

					BoundingBox<float> bb(1);
					bb.unserialize(is);
					QVertexAttrib<> qCoeffs(_q, bb, nCoeffsVar, 1);
					qCoeffs.setRow() = Eigen::Map<Eigen::VectorXi>(&vQuantCoefficients[nCoeffsVarStart], nCoeffsVar);
					qCoeffs.dequantize(&coefficients[nCoeffsVarStart]);
				}

			}

			// re-obtain geometry
			for (int i = 0; i < nCoeffs; ++i) {
				for (int j = 0; j < m_VertCount; ++j) {
					m_Vertices[j * 3 + offset] += s.EigenVectors(j, i) * coefficients[i];
				}
			}
		}

	} else {

#ifdef HAVE_METIS_H
		std::vector<MetisPartition> partList = Metis::Split(&m_Faces[0], m_FaceCount, m_VertCount, splitThreshold);

		int nParts = partList.size();
		std::vector<int> coeffsPerPart(nParts);

		int hufSize;
		is.read((char*)&hufSize, sizeof(int));
		std::vector<unsigned char> tmpBuf(hufSize);
		is.read((char*)&tmpBuf[0], hufSize * sizeof(unsigned char));

		Huffman_Uncompress((unsigned char*)&tmpBuf[0], (unsigned char*)&coeffsPerPart[0], hufSize, nParts * sizeof(int));

		int MetisCoeffSize = *std::max_element(coeffsPerPart.begin(), coeffsPerPart.end()) * nParts;
		std::vector<float> MetisCoefficients(3 * MetisCoeffSize);
		float *MetisCoeffsPtr = &MetisCoefficients[0];

		for (int i = 0; i < 3; ++i) {

			std::vector<int> vQuantCoefficients(MetisCoeffSize);

#ifdef USE_HUFFMAN_CODER
			int hSize;
			is.read((char*)&hSize, sizeof(int));
			std::vector<char> hCoeffs(hSize);
			is.read(&hCoeffs[0], hSize);

			Huffman_Uncompress((unsigned char*)&hCoeffs[0],
							   (unsigned char*)&vQuantCoefficients[0],
							   hSize, MetisCoeffSize * sizeof(int));
#else
			RangeDec r(is);
			VBE::action::in::Range<RangeDec, MoffatModel> aritdecoder(1 << ((q == 0)? (16 + 1) : q), &r);
			r.start();
			for (int i = 0; i < MetisCoeffSize; ++i)
				aritdecoder(vQuantCoefficients[i]);
			r.done();
#endif

			if (q != 0) { // uniform quantization

				BoundingBox<float> bb(1);
				bb.unserialize(is);

				QVertexAttrib<> qCoeffs(q, bb, MetisCoeffSize, 1);
				qCoeffs.setRow() = Eigen::Map<Eigen::VectorXi>(&vQuantCoefficients[0], MetisCoeffSize);
				qCoeffs.dequantize(MetisCoeffsPtr);

			} else {

				// non uniform quantization
				// | 5% | 10% | 10% | 75% |   ( % spectral coefficients )
				// +----+-----+-----+-----+
				// | 12 | 10  | 8   |  4  |   ( q )
				const int coeffVals[] = { 5, 10, 10, 75 };
				const int qVals[] = { 12, 10,  8,  4 };

				for (int i = 0; i < 4; ++i) {

					int nCoeffsVarStart = 0;
					if (i != 0) {
						for (int j = 0; j < i; ++j)
							nCoeffsVarStart += coeffVals[j];
						nCoeffsVarStart *= (float)MetisCoeffSize * 0.01;
					}

					int nCoeffsVar = (float)MetisCoeffSize * 0.01 * coeffVals[i];
					int _q = qVals[i];

					BoundingBox<float> bb(1);
					bb.unserialize(is);
					QVertexAttrib<> qCoeffs(_q, bb, nCoeffsVar, 1);
					qCoeffs.setRow() = Eigen::Map<Eigen::VectorXi>(&vQuantCoefficients[nCoeffsVarStart], nCoeffsVar);
					qCoeffs.dequantize(MetisCoeffsPtr + nCoeffsVarStart);
				}

			}
			MetisCoeffsPtr += MetisCoeffSize;
		}
		MetisCoeffsPtr = &MetisCoefficients[0];

		EigenSolver s;

#ifdef USE_OPENMP
		#pragma omp parallel for
#endif
		for (unsigned int p = 0; p < nParts; ++p) {

			int vCount = partList[p].VertexCount();

/*#ifdef USE_OPENMP
			float *MetisCoeffsPrivatePtr = &MetisCoefficients[0] + ((p != 0)?
										   Eigen::Map<Eigen::VectorXi>(&coeffsPerPart[0], p).sum() :
										   0);
#else*/
			float *MetisCoeffsPrivatePtr = MetisCoeffsPtr;
//#endif

			Eigen::MatrixXf L(vCount, vCount);
			Metis::BuildLaplacian(L, partList[p], m_Faces);

			Eigen::MatrixXf *ev = s.solve(L);

			int nCoeffsPart = coeffsPerPart[p];
			for (int offset = 0; offset < 3; ++offset) {
				for (int i = 0; i < nCoeffsPart; ++i) {
					for (int j = 0; j < vCount; ++j) {
						int realj = partList[p].GetVertexOriginalIndex(j);
						m_Vertices.at(realj * 3 + offset) += (*ev)(j, i) * MetisCoeffsPrivatePtr[(offset * MetisCoeffSize) + (nParts * i + p)]; //*(MetisCoeffsPrivatePtr + (offset * MetisCoeffSize) + i);
					}
				}
			}

			delete ev;
/*
#ifndef USE_OPENMP
			MetisCoeffsPtr += coeffsPerPart[p];
#endif
*/
		}

#else
		throw std::logic_error("Splitted mesh but MeTiS support not compiled in!");
#endif
	}
}
