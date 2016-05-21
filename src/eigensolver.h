#pragma once
#ifndef EIGENSOLVER_H
#define EIGENSOLVER_H

#include <stdexcept>
#include <Eigen/Core>
#include <Eigen/Sparse>
#ifdef USE_CUDA
 #include <cuda_runtime.h>
#endif

class EigenSolver
{
public:
	EigenSolver();
	EigenSolver(Eigen::DynamicSparseMatrix<float>&);
	const Eigen::MatrixXf& EigenVectors()  { return m_EigenVectors;       }
	const float EigenVectors(int i, int j) { return m_EigenVectors(i, j); }
	const float EigenValues(int i)         { return m_EigenValues(i);     }
	Eigen::MatrixXf* solve(Eigen::MatrixXf &A) const;
#ifdef USE_CUDA
	void setMatrixSize(const int matOrder);
	bool fillBuffer(const Eigen::MatrixXf &A);
	void cudaSolve();
	bool getNextResult(float *eigenVectors, unsigned *matrixOrder) const;
#endif

private:
	void solve();

#ifdef USE_CUDA
	float *gpuMatBuffer;
	float *gpuResBuffer;
	float *resBuffer;
	unsigned *matSizes;
	unsigned *gpuMatSizes;
	unsigned matCount;
	unsigned matIdx;
	unsigned bufSize;
#endif

	int m_ProblemSize;
	Eigen::SparseMatrix<float, Eigen::RowMajor> m_Matrix;
	Eigen::MatrixXf m_EigenVectors;
	Eigen::VectorXf m_EigenValues;
};

#endif // EIGENSOLVER_H
