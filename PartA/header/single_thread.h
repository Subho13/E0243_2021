// Optimize this function

void singleThread(int N, int *matA, int *matB, int *output)
{
    // enforce N to be power of 2 and greater than 2
    assert(N>=4 and N == ( N &~ (N-1)));
    for(int rowA = 0; rowA < N; rowA++){
        for(int colSetB = 0; colSetB < N; colSetB += 2){
            int sum = 0;
            if(rowA & 1){    
                // handle odd rows in matA
                
                // iterate over even positions in rowA
                // and odd positions in column colSetB in matB
                for(int indexA = rowA*N, indexB = colSetB+N; indexA < (rowA+1)*N; 
                        indexA += 2, indexB +=(N<<1))
                    sum += matA[indexA] * matB[indexB];
                
                // iterate over odd positions in rowA
                // and even positions in column colSetB+1 in matB
                for(int indexA = rowA*N+1, indexB = colSetB+1; indexA < (rowA+1)*N; 
                        indexA += 2, indexB += (N<<1))
                    sum += matA[indexA] * matB[indexB];
            
            } else {
                // handle even rows in matA

                // iterate over even positions in rowA
                // and odd positions in column colSetB+1 in matB
                for(int indexA = rowA*N, indexB = colSetB+1+N; indexA < (rowA+1)*N; 
                        indexA += 2, indexB += (N<<1))
                    sum += matA[indexA] * matB[indexB];

                // iterate over odd positions in rowA
                // and even positions in column colSetB in matB
                for(int indexA = rowA*N+1, indexB = colSetB; indexA < (rowA+1)*N; 
                        indexA += 2, indexB += (N<<1))
                    sum += matA[indexA] * matB[indexB];
            }
            
            // compute output indices
            int rowC = rowA>>1;
            int colC = (colSetB>>1) + (N>>1) * (rowA&1);
            int indexC = rowC * N + colC;
            output[indexC] = sum;
        }
    }
}


