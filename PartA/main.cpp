#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <chrono>
#include <fstream>
#include <assert.h>

using namespace std;

#define TIME_NOW std::chrono::high_resolution_clock::now()
#define TIME_DIFF(gran, start, end) std::chrono::duration_cast<gran>(end - start).count()

#include "single_thread.h"
#include "multi_thread.h"

// Used to cross-check answer. DO NOT MODIFY!
void reference(int N, int *matA, int *matB, int *output)
{
    // enforce N to be power of 2 and greater than 2
    assert( N>=4 and N == ( N &~ (N-1)));
    for(int rowA = 0; rowA < N; rowA++){
        for(int colSetB = 0; colSetB < N; colSetB += 2){
            int sum = 0;
            if(rowA & 1) {    
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

int main(int argc, char *argv[])
{
    // Input size of square matrices
    int N;
    string file_name; 
    if (argc < 2) 
        file_name = "data/input_8192.in"; 
    else 
        file_name = argv[1]; 
    ifstream input_file; 
    input_file.open(file_name); 
    input_file >> N;
    cout << "Input matrix of size " << N << "\n";
    
    // Input matrix A
    int *matA = new int[N * N];
    for(int i = 0; i < N; ++i)
        for(int j = 0; j < N; ++j)
            input_file >> matA[i * N + j];

    // Input matrix B
    int *matB = new int[N * N];
    for(int i = 0; i < N; ++i)
        for(int j = 0; j < N; ++j)
            input_file >> matB[i * N + j];
    
    // Untimed, warmup caches and TLB
    int *output_reference = new int[N*(N>>1)];
    reference(N, matA, matB, output_reference);
    
    // Execute reference program
    auto begin = TIME_NOW;
    reference(N, matA, matB, output_reference);
    auto end = TIME_NOW;
    cout << "Reference execution time: " << 
        (double)TIME_DIFF(std::chrono::microseconds, begin, end) / 1000.0 << " ms\n";    

    if(N<=8){
        cerr << "matA: " << endl;
        for(int i=0; i<(N); i++){
            for(int j=0; j<N; j++){
                cerr << matA[i*N+j] << "\t";
            }
            cerr << endl;
        }
        cerr << "matB: " << endl;
        for(int i=0; i<(N); i++){
            for(int j=0; j<N; j++){
                cerr << matB[i*N+j] << "\t";
            }
            cerr << endl;
        }
        cerr << "output_reference: " << endl;
        for(int i=0; i<(N>>1); i++){
            for(int j=0; j<N; j++){
                cerr << output_reference[i*N+j] << "\t";
            }
            cerr << endl;
        }
    }

    // Execute single thread
    int *output_single = new int[N*(N>>1)];
    begin = TIME_NOW;
    singleThread(N, matA, matB, output_single);
    end = TIME_NOW;
    cout << "Single thread execution time: " << 
        (double)TIME_DIFF(std::chrono::microseconds, begin, end) / 1000.0 << " ms\n";
    
    for(int i = 0; i < (N*(N>>1)); ++i)
        if(output_single[i] != output_reference[i]) {
            cout << "Mismatch at " << i << "\n";
            exit(0);
        }
    
    // Execute multi-thread
    int *output_multi = new int[N*(N>>1)];
    begin = TIME_NOW;
    multiThread(N, matA, matB, output_multi);
    end = TIME_NOW;
    cout << "Multi-threaded execution time: " << 
        (double)TIME_DIFF(std::chrono::microseconds, begin, end) / 1000.0 << " ms\n";
    
    for(int i = 0; i < (N*(N>>1)); ++i)
        if(output_multi[i] != output_reference[i]) {
            cout << "Mismatch at " << i << "\n";
            exit(0);
        }

    input_file.close(); 
    return 0; 
}
