__global__ void matmul(int *matA, int *matB, int *output, int N) {
    int rowA = blockIdx.x / N;
    int rowB = blockIdx.x % N;
    int colSetB = threadIdx.x;

    int oddRowA = rowA & 1;
    int oddRowB = rowB & 1;
    int colA = (colSetB << 1) + (!oddRowB);
    int colB = (colSetB << 1) + (oddRowA ^ oddRowB);
    int rowC = rowA >> 1;
    int colC = colSetB + oddRowA * (N >> 1);

    output[rowC * N + colC] += matA[rowA * N + colA] + matB[rowB * N + colB];
}

__global__ void matmul2(int *dmatA, int *dmatB, int *doutput, int N) {
    int y = blockDim.x * blockIdx.x + threadIdx.x;
    if (y>=N*N/2) return;
    int halfN = N >> 1;
    int row = y / N;
    int col = y % N;

    int rowA = row << 1;
    int colsetB = col;
    if (col >= halfN) {
        rowA += 1;
        colsetB -= halfN;
    }
    
    doutput[y] = 0;
    if (rowA & 1) { // odd
        for (int i = 0; i < halfN; i++) {
            int colA = i << 1;
            int colB = colsetB << 1;
            int rowB = (i << 1) | 1;
            doutput[y] += dmatA[rowA * N + colA] * dmatB[rowB * N + colB];

            colA++;
            colB++;
            rowB--;
            doutput[y] += dmatA[rowA * N + colA] * dmatB[rowB * N + colB];
        }
    } else { // even
        for (int i = 0; i < halfN; i++) {
            int colA = i << 1;
            int colB = (colsetB << 1) | 1;
            int rowB = (i << 1) + 1;
            doutput[y] += dmatA[rowA * N + colA] * dmatB[rowB * N + colB];

            colA++;
            colB--;
            rowB--;
            doutput[y] += dmatA[rowA * N + colA] * dmatB[rowB * N + colB];
        }
    }
}

#define TILE_SIZE 16
__global__ void matmul3(int *matA, int *matB, int *output, int N) {
    int rowC = blockDim.x * blockIdx.x + threadIdx.x;
    int colC = blockDim.y * blockIdx.y + threadIdx.y;

    if (rowC >= N || colC >= N) return;

    int tiles = N / TILE_SIZE;
    int halfN = (N >> 1);
    int oddRowA = (colC >= halfN);
    int rowA = (rowC << 1) + oddRowA;
    int colSetB = colC - oddRowA * halfN;
    int colB = colSetB << 1;

    __shared__ int A[TILE_SIZE][TILE_SIZE];
    __shared__ int B1[TILE_SIZE][TILE_SIZE];
    __shared__ int B2[TILE_SIZE][TILE_SIZE];

    int sum = 0;
    for (int i = 0; i < tiles; i++) {
        int tileIdx = TILE_SIZE * i;
        int colA = tileIdx + threadIdx.y;
        int rowB = tileIdx + threadIdx.x;

         A[threadIdx.x][threadIdx.y] = matA[rowA * N + colA];
        B1[threadIdx.x][threadIdx.y] = matB[rowB * N + colB];
        B2[threadIdx.x][threadIdx.y] = matB[rowB * N + colB + 1];
        __syncthreads();

        for (int j = 0; j < (TILE_SIZE >> 1); j++) {
            if (oddRowA) {
                sum += A[threadIdx.x][(j << 1)] *
                       B1[(j << 1) + 1][threadIdx.y];
                sum += A[threadIdx.x][(j << 1) + 1] *
                       B2[(j << 1)][threadIdx.y];
            } else {
                sum += A[threadIdx.x][(j << 1)] *
                       B2[(j << 1) + 1][threadIdx.y];
                sum += A[threadIdx.x][(j << 1) + 1] *
                       B1[(j << 1)][threadIdx.y];
            }
        }
        __syncthreads();
    }

    output[rowC * N + colC] = sum;
}

void gpuThread(int N, int *matA, int *matB, int *output)
{
    int totSize = N * N;
    int totSizeHalf = totSize >> 1;
    int blocks = N;
    int threads = N;
    int colSets = N >> 1;

    int *dmatA, *dmatB, *doutput;
    cudaMalloc((void **)&dmatA, sizeof(int)*totSize);
    cudaMalloc((void **)&dmatB, sizeof(int)*totSize);
    cudaMalloc((void **)&doutput, sizeof(int)*totSizeHalf);

    cudaMemcpy(dmatA, matA, sizeof(int)*totSize, cudaMemcpyHostToDevice);
    cudaMemcpy(dmatB, matB, sizeof(int)*totSize, cudaMemcpyHostToDevice);

/*
    // ATTEMPT 1:
    blocks = N * N;
    
    cudaMemset(doutput, 0, sizeof(int)*totSizeHalf);
    matmul<<<blocks, colSets>>>(dmatA, dmatB, doutput, N);
*/
/*
    // ATTEMPT 2:
    matmul<<<totSizeHalf/2048 + 1, 2048>>>(dmatA, dmatB, doutput, N);
*/

    // ATTEMPT 3:
    // Assumption: dimension is power
    // of 2 (greater than 16)
    int dimX = N / TILE_SIZE;
    int dimY = colSets / TILE_SIZE;
    dim3 blocks(dimY, dimX, 1);
    dim3 threads(TILE_SIZE, TILE_SIZE, 1);
    matmul3<<<blocks, threads>>>(dmatA, dmatB, doutput, N);
    
    cudaMemcpy(output, doutput, sizeof(int)*totSizeHalf, cudaMemcpyDeviceToHost);
    cudaFree(dmatA);
    cudaFree(dmatB);
    cudaFree(doutput);
}
