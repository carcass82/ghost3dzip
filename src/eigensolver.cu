#include <stdio.h>
#include <cuda_runtime.h>
#include <float.h>

__device__ void swap(float *a, float *b)
{
	float tmp = *a;
	*a = *b;
	*b = tmp;
}

extern __shared__ float BlockShMem[];
__global__ void CUDAJacobi(float *Aptr, float *Zptr, const unsigned int *Mptr, const int Nmat)
{
	const int N = sqrtf(Mptr[blockIdx.x]);
	float *A = Aptr + Mptr[blockIdx.x];
	float *Z = Zptr + Mptr[blockIdx.x];

	float *z = (float*)&BlockShMem[0];
	float *b = (float*)&BlockShMem[N];
	float *D = (float*)&BlockShMem[2 * N];

	// initialize Z as identity matrix
	//            D with A main diagonal
	//            b = D
	//            and z = 0
	for (int i = 0; i < N; ++i) {

		D[i] = A[i * N + i];
		b[i] = D[i];
		z[i] = 0.0;

		for (int j = 0; j < N; ++j) {
			Z[i * N + j] = (i == j)? 1.0 : 0.0;
		}
	}

	float g, h, t, theta, c, s, tau;
	const float EPSILON = FLT_EPSILON;
	int iteration = -1;
	const int MAX_ITERATIONS = 50;

	for (iteration = 0; iteration < MAX_ITERATIONS; ++iteration) {

		// convergence test: if we have a diagonal matrix (to machine precision) we're done
		float sm = 0.0;
		for (int i = 0; i < N - 1; ++i)
			for (int j = i + 1; j < N; ++j)
				sm += fabs(A[i * N + j]);

		if (sm == 0.0f)
			break;

		for (int ip = 0; ip < N - 1; ++ip) {
			for (int iq = ip + 1; iq < N; ++iq) {

				float Apq = A[ip * N + iq];
				g = 100.0f * fabsf(Apq);

				if (iteration > 4 && (g <= EPSILON * fabsf(D[ip])) && (g <= EPSILON * fabsf(D[iq]))) {

					A[ip * N + iq] = 0.0f;

				} else if (fabsf(Apq) > 0.0f) {

					h = D[iq] - D[ip];
					if (g <= EPSILON * fabsf(h)) {
						t = Apq / h;
					} else {
						theta = 0.5f * h / Apq;
						t = 1.0f / (fabsf(theta) + sqrtf(1.0f + theta * theta));
						if (theta < 0.0f)
							t = -t;
					}
					c = 1.0f / sqrtf(1 + t * t);
					s = t * c;
					tau = s / (1.0f + c);
					h = t * Apq;

					z[ip] -= h;
					D[ip] -= h;

					z[iq] += h;
					D[iq] += h;

					A[ip * N + iq] = 0.0f;

					// case of rotations 0 <= j < p
					for (int j = 0; j < ip; ++j) {
						float Aij = A[j * N + ip];
						float Akl = A[j * N + iq];
						A[j * N + ip] = Aij - s * (Akl + Aij * tau);
						A[j * N + iq] = Akl + s * (Aij - Akl * tau);
					}

					// case of rotations p <= j < q
					for (int j = ip + 1; j < iq; ++j) {
						float Aij = A[ip * N + j];
						float Akl = A[j * N + iq];
						A[ip * N + j] = Aij - s * (Akl + Aij * tau);
						A[j * N + iq] = Akl + s * (Aij - Akl * tau);
					}

					// case of rotations q <= j < n
					for (int j = iq + 1; j < N; ++j) {
						float Aij = A[ip * N + j];
						float Akl = A[iq * N + j];
						A[ip * N + j] = Aij - s * (Akl + Aij * tau);
						A[iq * N + j] = Akl + s * (Aij - Akl * tau);
					}

					// accumulate rotations for eigenvectors
					for (int j = 0; j < N; ++j) {
						float Aij = Z[j * N + ip];
						float Akl = Z[j * N + iq];
						Z[j * N + ip] = Aij - s * (Akl + Aij * tau);
						Z[j * N + iq] = Akl + s * (Aij - Akl * tau);
					}
				}
			}
		}

		for (int i = 0; i < N; ++i) {
			b[i] += z[i];
			D[i] = b[i];
			z[i] = 0.0;
		}
	}

	if (iteration < MAX_ITERATIONS) {

		// converged, now put eigenvalues in ascending order
		int min_idx;
		for (int i = 0; i < N; ++i) {
			min_idx = i;
			for (int j = i + 1; j < N; ++j)
				if (D[j] < D[min_idx])
					min_idx = j;

			if (i != min_idx)
				swap(&D[i], &D[min_idx]);
		}

	} else {

		for (int i = 0; i < N; ++i)
			D[i] = -1.0f;

	}
}


extern "C"
void eigenpairs_gpu(float *gpuA, float *gpuZ, const unsigned MaxMatrixSize, const unsigned *gpuMatrixSize, const int MatrixCount)
{
	dim3 gridDim(MatrixCount, 1);
	dim3 blockDim(1, 1);
	int shMemSize = 3 * MaxMatrixSize * sizeof(float);

	CUDAJacobi<<<gridDim, blockDim, shMemSize>>>(gpuA, gpuZ, gpuMatrixSize, MatrixCount);
}
