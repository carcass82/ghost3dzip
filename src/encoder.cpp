#include "encoder.h"

Encoder::Encoder(float *v, unsigned *f, int vsize, int fsize)
{
	m_Vertices = v;
	m_Faces = f;

	m_VertCount = vsize / 3;
	m_FaceCount = fsize / 3;
}

void Encoder::Encode(std::ostream &os, float coeffsRatio, unsigned short splitThreshold, unsigned char q)
{
	// output number of coefficients
	int nCoeffs = (float)m_VertCount * 0.01f * coeffsRatio;
	os.write((char*)&nCoeffs, sizeof(int));

	// output split threshold
	os.write((char*)&splitThreshold, sizeof(unsigned short));

	// output quantization value
	os.write((char*)&q, sizeof(unsigned char));

	if (splitThreshold == 0) {

		Eigen::DynamicSparseMatrix<float> L(m_VertCount, m_VertCount);
		Utils::BuildLaplacian(L, m_Faces, m_FaceCount);
		EigenSolver s(L);

		// compute and write (only nCoeffs) coefficients
		std::vector<float> coefficients(nCoeffs);
		for (int offset = 0; offset < 3; ++offset) {

			for (int i = 0; i < nCoeffs; ++i) {
				coefficients[i] = 0.0f;
				for (int j = 0; j < m_VertCount; ++j) {
					coefficients[i] += s.EigenVectors(j, i) * m_Vertices[j * 3 + offset];
				}
			}


			/*
			if (offset == 0) {
				std::ofstream oaspd("fuffax");
				for (int i = 0; i < nCoeffs; ++i)
					oaspd << i << "\t" << coefficients[i] << "\n";
				oaspd.close();
			}
			if (offset == 1) {
				std::ofstream oaspd("fuffay");
				for (int i = 0; i < nCoeffs; ++i)
					oaspd << i << "\t" << coefficients[i] << "\n";
				oaspd.close();
			}
			if (offset == 2) {
				std::ofstream oaspd("fuffaz");
				for (int i = 0; i < nCoeffs; ++i)
					oaspd << i << "\t" << coefficients[i] << "\n";
				oaspd.close();
			}*/

			std::vector<int> vQuantCoefficients(nCoeffs);
			std::stringstream qBounds(std::ios::out);

			if (q != 0) { // uniform quantization

				QVertexAttrib<> qCoeffs(&coefficients[0], &coefficients[nCoeffs], 1, q);
				Eigen::Map<Eigen::VectorXi>(&vQuantCoefficients[0], vQuantCoefficients.size()) = qCoeffs.getRow();
				qCoeffs.getBoundingBox().serialize(qBounds);

			} else { // non uniform quantization

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

					QVertexAttrib<> qCoeffs(&coefficients[nCoeffsVarStart], &coefficients[nCoeffsVarStart + nCoeffsVar], 1, _q);
					int k = 0;
					for (int j = nCoeffsVarStart; j < nCoeffsVarStart + nCoeffsVar; ++j)
						vQuantCoefficients[j] = qCoeffs.getRow()[k++];
					qCoeffs.getBoundingBox().serialize(qBounds);
				}

			}

			/*
			if (offset == 0) {
				std::ofstream oaspd("fuffa_distquant_x");

				std::vector<int> piripi(1 << q, 0);
				for (int i = 0; i < nCoeffs; ++i)
					piripi[qCoeffs.getRow()[i]]++;


				for (int i = 0; i < piripi.size(); ++i)
					oaspd << i * qCoeffs.getBoundingBox().scale + qCoeffs.getBoundingBox().offset << "\t" << piripi[i] << "\n";

				oaspd.close();
			}*/


#ifdef USE_HUFFMAN_CODER
			// Huffman-encode quantized coefficients
			std::vector<unsigned char> res(nCoeffs * sizeof(int) + 384);
			int outSize = Huffman_Compress((unsigned char*)&vQuantCoefficients[0], &res[0], nCoeffs * sizeof(int));

			// output new size and data
			os.write((char*)&outSize, sizeof(int));
			os.write((char*)&res[0], outSize * sizeof(unsigned char));
#else
			RangeEnc r(os);
			VBE::action::out::Range<RangeEnc, MoffatModel> aritcoder(1 << ((q == 0)? (16 + 1) : q), &r);
			r.start(0, 0);
			for (int i = 0; i < nCoeffs; ++i)
				aritcoder(vQuantCoefficients[i]);
			r.done();
#endif
			os << qBounds.str();
		}

	} else {

#ifdef HAVE_METIS_H

		std::vector<MetisPartition> partList = Metis::Split(&m_Faces[0], m_FaceCount, m_VertCount, splitThreshold);

		int nParts = partList.size();
		std::vector<int> coeffsPerPart(nParts);

		int MetisCoeffSize = 0;
		for (int i = 0; i < nParts; ++i)
			coeffsPerPart[i] = (float)partList[i].VertexCount() * 0.01f * coeffsRatio;
		MetisCoeffSize = *std::max_element(coeffsPerPart.begin(), coeffsPerPart.end());
		MetisCoeffSize *= nParts;

		// Huffman-encode list of coeffs per part
		std::vector<unsigned char> tmpBuf(nParts * sizeof(int) + 384);
		int bufSize = Huffman_Compress((unsigned char*)&coeffsPerPart[0], &tmpBuf[0], nParts * sizeof(int));
		os.write((char*)&bufSize, sizeof(int));
		os.write((char*)&tmpBuf[0], bufSize * sizeof(unsigned char));

		std::vector<float> MetisCoefficients(3 * MetisCoeffSize);
		float *MetisCoeffsPtr = &MetisCoefficients[0];

		EigenSolver s;

#ifdef USE_CUDA
		int partStart = 0;
		int partEnd = 0;
		s.setMatrixSize(splitThreshold);

		while (partEnd < partList.size()) {
			bool bufferHasSpace = true;
			while (bufferHasSpace) {
				Eigen::MatrixXf L(partList[partEnd].VertexCount(), partList[partEnd].VertexCount());
				Metis::BuildLaplacian(L, partList[partEnd], m_Faces);
				if (!(bufferHasSpace = s.fillBuffer(L)))
					break;

				if (++partEnd == partList.size() - 1)
					break;
			}

			s.cudaSolve();

			int p = partStart;
			bool bufferHasResults = true;
			while (bufferHasResults) {
				float *evecs;
				unsigned mOrder;
				bufferHasResults = s.getNextResult(evecs, &mOrder);
				if (!bufferHasResults)
					break;

				Eigen::Map<Eigen::MatrixXf> ev(evecs, mOrder, mOrder);
				float *MetisCoeffsPrivatePtr = MetisCoeffsPtr;

				for (int offset = 0; offset < 3; ++offset) {
					for (int i = 0; i < coeffsPerPart[p]; ++i) {
						for (int j = 0; j < partList[p].VertexCount(); ++j) {
							int realj = partList[p].GetVertexOriginalIndex(j);
							*(MetisCoeffsPrivatePtr + (offset * MetisCoeffSize) + i) += ev(j, i) * m_Vertices[realj * 3 + offset];
						}
					}
				}
				MetisCoeffsPtr += coeffsPerPart[p];

				if (++p == partEnd)
					break;
			}

			partStart = partEnd + 1;
			partEnd = partStart;
		}

#else

#ifdef USE_OPENMP
		#pragma omp parallel for
#endif
		for (unsigned int p = 0; p < nParts; ++p) {

			int vCount = partList[p].VertexCount();
			Eigen::MatrixXf L(vCount, vCount);
			Metis::BuildLaplacian(L, partList[p], m_Faces);

			Eigen::MatrixXf *ev = s.solve(L);

			for (int offset = 0; offset < 3; ++offset) {
				for (int i = 0; i < coeffsPerPart[p]; ++i) {
					for (int j = 0; j < vCount; ++j) {
						int realj = partList[p].GetVertexOriginalIndex(j);
						MetisCoefficients[(offset * MetisCoeffSize) + (nParts * i + p)] += (*ev)(j, i) * m_Vertices[realj * 3 + offset];
					}
				}

				/*
				if (p == 4) {
					if (offset == 0) {
						std::ofstream oaspd("fuffax_part");
						for (int i = 0; i < MetisCoeffSize; ++i)
							oaspd << i << "\t" << MetisCoeffsPrivatePtr[i] << "\n";
						oaspd.close();
					}
					if (offset == 1) {
						std::ofstream oaspd("fuffay_part");
						for (int i = 0; i < MetisCoeffSize; ++i)
							oaspd << i << "\t" << MetisCoeffsPrivatePtr[MetisCoeffSize + i] << "\n";
						oaspd.close();
					}
					if (offset == 2) {
						std::ofstream oaspd("fuffaz_part");
						for (int i = 0; i < MetisCoeffSize; ++i)
							oaspd << i << "\t" << MetisCoeffsPrivatePtr[2 * MetisCoeffSize + i] << "\n";
						oaspd.close();
					}
				}
				*/

			}

			delete ev;
		}

#endif

		MetisCoeffsPtr = &MetisCoefficients[0];
		for (int i = 0; i < 3; ++i) {

			/*
			if (i == 0) {
				std::ofstream oaspd("fuffa_metis_x");
				for (int j = 0; j < MetisCoeffSize; ++j)
					oaspd << j << "\t" << MetisCoeffsPtr[j] << "\n";
				oaspd.close();
			}
			*/

			std::vector<int> vQuantCoefficients(MetisCoeffSize);
			std::stringstream qBounds(std::ios::out);

			if (q != 0) { // uniform quantization

				QVertexAttrib<> qCoeffs(MetisCoeffsPtr, MetisCoeffsPtr + MetisCoeffSize, 1, q);
				Eigen::Map<Eigen::VectorXi>(&vQuantCoefficients[0], MetisCoeffSize) = qCoeffs.getRow();
				qCoeffs.getBoundingBox().serialize(qBounds);

			} else { // non uniform quantization

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

					QVertexAttrib<> qCoeffs(MetisCoeffsPtr + nCoeffsVarStart, MetisCoeffsPtr + nCoeffsVarStart + nCoeffsVar, 1, _q);
					int k = 0;
					for (int j = nCoeffsVarStart; j < nCoeffsVarStart + nCoeffsVar; ++j)
						vQuantCoefficients[j] = qCoeffs.getRow()[k++];
					qCoeffs.getBoundingBox().serialize(qBounds);
				}

			}

#ifdef USE_HUFFMAN_CODER
			// Huffman-encode quantized coefficients
			std::vector<unsigned char> res(MetisCoeffSize * sizeof(int) + 384);
			int outSize = Huffman_Compress((unsigned char*)&vQuantCoefficients[0], &res[0], MetisCoeffSize * sizeof(int));

			// output new size and data
			os.write((char*)&outSize, sizeof(int));
			os.write((char*)&res[0], outSize * sizeof(unsigned char));
#else
			RangeEnc r(os);
			VBE::action::out::Range<RangeEnc, MoffatModel> aritcoder(1 << ((q == 0)? (16 + 1) : q), &r);
			r.start(0, 0);
			for (int i = 0; i < MetisCoeffSize; ++i)
				aritcoder(vQuantCoefficients[i]);
			r.done();
#endif
			os << qBounds.str();

			MetisCoeffsPtr += MetisCoeffSize;
		}

#else
		// this shouldn't happen
		throw std::logic_error("Mesh splitting requested but MeTiS support not compiled in!");
#endif
	}
}
