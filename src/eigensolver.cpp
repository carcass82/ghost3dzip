#include "eigensolver.h"

extern "C"
void ssyevr_(char *jobz, char *range, char *uplo, int *n, float *a, int *lda, float *vl,
			 float *vu, int *il, int *iu, float *abstol, int *m, float *w, float *z, int *ldz,
			 int * isuppz, float *work, int *lwork, int *iwork, int *liwork, int *info);


EigenSolver::EigenSolver()
	: m_ProblemSize(0)
{}

EigenSolver::EigenSolver(Eigen::DynamicSparseMatrix<float> &m)
	: m_Matrix(m), m_ProblemSize(m.rows())
{
	solve();
}

void EigenSolver::solve()
{
	m_EigenValues.setZero(m_ProblemSize);
	m_EigenVectors.setZero(m_ProblemSize, m_ProblemSize);

	Eigen::MatrixXf A = m_Matrix * Eigen::MatrixXf::Identity(m_ProblemSize, m_ProblemSize);

	char jobz = 'V';
	char range = 'A';
	char uplo = 'U';
	int n = m_ProblemSize;
	int m;
	float vl = 0.0f, vu = 0.0f;
	int il = 0, iu = 0;
	float abstol = 0.0f;
	int *isuppz = new int[2 * n];
	float worktest[1];
	int lwork = -1;
	int iworktest[1];
	int liwork = -1;
	int info;

	// first lapack call: determine required workspace (lwork = -1)
	ssyevr_(&jobz, &range, &uplo, &n, A.data(), &n, &vl, &vu, &il, &iu, &abstol, &m, m_EigenValues.data(), m_EigenVectors.data(), &n, isuppz, worktest, &lwork, iworktest, &liwork, &info);

	// allocate required workspace
	lwork = (int)worktest[0];
	float *work = new float[lwork];

	liwork = (int)iworktest[0];
	int *iwork = new int[liwork];

	// solve eigenproblem
	ssyevr_(&jobz, &range, &uplo, &n, A.data(), &n, &vl, &vu, &il, &iu, &abstol, &m, m_EigenValues.data(), m_EigenVectors.data(), &n, isuppz, work, &lwork, iwork, &liwork, &info);

	if (info != 0)
		throw std::runtime_error("ssyevr routine could not solve eigenproblem!");

	delete[] isuppz;
	delete[] work;
	delete[] iwork;
}

Eigen::MatrixXf* EigenSolver::solve(Eigen::MatrixXf &A) const
{
	char jobz = 'V';
	char range = 'A';
	char uplo = 'U';
	int n = A.outerSize();
	int m;
	float vl = 0.0f, vu = 0.0f;
	int il = 0, iu = 0;
	float abstol = 0.0f;
	int *isuppz = new int[2 * n];
	float worktest[1];
	int lwork = -1;
	int iworktest[1];
	int liwork = -1;
	int info;
	float *evals = new float[n];
	Eigen::MatrixXf *evecs = new Eigen::MatrixXf(n, n);

	// first lapack call: determine required workspace (lwork = -1)
	ssyevr_(&jobz, &range, &uplo, &n, A.data(), &n, &vl, &vu, &il, &iu, &abstol, &m, evals, evecs->data(), &n, isuppz, worktest, &lwork, iworktest, &liwork, &info);

	// allocate required workspace
	lwork = (int)worktest[0];
	float *work = new float[lwork];

	liwork = (int)iworktest[0];
	int *iwork = new int[liwork];

	// solve eigenproblem
	ssyevr_(&jobz, &range, &uplo, &n, A.data(), &n, &vl, &vu, &il, &iu, &abstol, &m, evals, evecs->data(), &n, isuppz, work, &lwork, iwork, &liwork, &info);

	if (info != 0)
		throw std::runtime_error("ssyevr routine could not solve eigenproblem!");

	delete[] evals;
	delete[] isuppz;
	delete[] work;
	delete[] iwork;

	return evecs;
}

#ifdef USE_CUDA
void EigenSolver::setMatrixSize(const int matOrder)
{
	cudaDeviceProp deviceProp;
	cudaGetDeviceProperties(&deviceProp, 0);

	// set host-device memory mapping capability
	cudaSetDeviceFlags(cudaDeviceMapHost);

	// cuda specs allow 8 concurrent blocks per multiprocessor
	matCount = 8 * deviceProp.multiProcessorCount;
	bufSize = (matOrder + 4) * (matOrder + 4) * sizeof(float) * matCount;

	// allocate device buffer for laplacian matrices
	cudaMalloc((void**)&gpuMatBuffer, bufSize);

	// and host buffer for results
	cudaHostAlloc((void**)&resBuffer, bufSize, cudaHostAllocMapped);
	cudaHostGetDevicePointer((void**)&gpuResBuffer, resBuffer, 0);

	// matrices are not guaranteed to have the same size
	cudaHostAlloc((void**)&matSizes, matCount, cudaHostAllocMapped);
	cudaHostGetDevicePointer((void**)&gpuMatSizes, matSizes, 0);

	matSizes[0] = 0;
	matIdx = 0;
}

bool EigenSolver::fillBuffer(const Eigen::MatrixXf &A)
{
	if (matIdx == matCount)
		return false;

	cudaMemcpy(gpuMatBuffer + matSizes[matIdx], A.data(), A.size() * sizeof(float), cudaMemcpyHostToDevice);
	Eigen::Map<Eigen::MatrixXf>(resBuffer, A.rows(), A.cols()).setIdentity();
	matSizes[matIdx + 1] = matSizes[matIdx] + A.size();
	++matIdx;

	return true;
}

extern "C" void eigenpairs_gpu(float *A, float *Z, const unsigned MaxMatrixSize, const unsigned *MatrixSize, const int MatrixCount);
void EigenSolver::cudaSolve()
{
	// compute max matrix dimension in buffer
	int mmax = matSizes[1];
	for (int i = 2; i < matCount; ++i)
		if (matSizes[i] - matSizes[i - 1] > mmax)
			mmax = matSizes[i] - matSizes[i - 1];

	eigenpairs_gpu(gpuMatBuffer, gpuResBuffer, sqrtf(mmax), gpuMatSizes, matCount);
	matIdx = 0;
}

bool EigenSolver::getNextResult(float *eigenVectors, unsigned *matrixOrder) const
{
	if (matIdx == 0)
		cudaThreadSynchronize(); // ensure kernel has finished

	if (matIdx == matCount)
		return false;

	eigenVectors = resBuffer + matSizes[matIdx];
	*matrixOrder = matSizes[matIdx + 1] - matSizes[matIdx];

	return true;
}
#endif
