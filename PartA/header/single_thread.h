void singleThread(int N, int *matA, int *matB, int *output)
{
    // enforce N to be power of 2 and greater than 2
    assert(N>=4 and N == ( N &~ (N-1)));
    for (int rowA = 0; rowA < N; rowA++) {
    	int rowC = rowA >> 1;
    	int tempIndexA = rowA * N;
    	int tempIndexC = rowC * N + (N >> 1) * (rowA & 1);

		// Clear output matrix
		for (int i = 0; i < (N>>1); ++i) {
			output[tempIndexC + i] = 0;
		}
    	for (int rowB = 0; rowB < N; rowB++) {
			int colA = rowB ^ 1;
			int colB = (rowA + rowB) & 1;
			int indexA = tempIndexA + colA;

			for (int i = colB; i < N; i+=2) {
				int indexB = rowB * N + i;
				output[tempIndexC + (i >> 1)] += matA[indexA] * matB[indexB];
			}
    	}
    }
}
