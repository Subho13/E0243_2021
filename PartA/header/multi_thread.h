#include <pthread.h>

#define NUM_THREADS 2
typedef struct {
    int start, end, N;
    int *matA, *matB, *matC;
} Args;

void *cmmThread(void* param) {
    Args* args = (Args*) param;

    for (int rowA = 0; rowA < args->N; ++rowA) {
        int evenRowA = rowA & 1;
        int tempIndexC = ((rowA >> 1) * (args->N)) + ((args->N) >> 1) * evenRowA;

        for (int i = args->start; i < args->end; ++i) {
            args->matC[tempIndexC + i] = 0;
        }

        for (int rowB = 0; rowB < args->N; ++rowB) {
            int indexA = rowA * (args->N) + (rowB ^ 1);
            int tempIndexB = rowB * (args->N);
            int tempColB = evenRowA ^ (rowB & 1);

            for (int colSetB = args->start; colSetB < args->end; ++colSetB) {
                int colB = (colSetB << 1) | tempColB;
                args->matC[tempIndexC + colSetB] += (args->matA[indexA]) * (args->matB[tempIndexB + colB]);
            }
        }
    }

    pthread_exit(NULL);
}

void multiThread(int N, int *matA, int *matB, int *output)
{
    int columnSets = N >> 1;
    int colSetsPerThread = columnSets / NUM_THREADS;
    pthread_t threads[NUM_THREADS];
    Args args [NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        args[i].start = colSetsPerThread * i;
        args[i].end = args[i].start + colSetsPerThread;
        args[i].N = N;
        args[i].matA = matA;
        args[i].matB = matB;
        args[i].matC = output;
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], NULL, cmmThread, (void*)(args + i));
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }
}
