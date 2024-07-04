#include <stdio.h>
#include <time.h>

#include "smolar.h"

int main()
{
    int shape_a[] = {1000, 1000};
    int shape_b[] = {1000, 1000};

    int num_runs = 5;
    clock_t start, end;
    float cpu_time_used, total_time = 0.0f;

    Array *a = smReshapeNew(smArange(1, (1000 * 1000) + 1, 1), shape_a, 2);
    Array *b = smReshapeNew(smArange(1, (1000 * 1000) + 1, 1), shape_b, 2);

    printf("\nA shape:\n");
    __printArrayInternals__(a, a->shape);
    printf("B shape:\n");
    __printArrayInternals__(b, b->shape);

    printf("benchmarking...\n");
    for(int i=0; i<num_runs; i++)
    {
        start = clock();
        Array *res = smMatMul(a, b);
        end = clock();

        printf("completed run %d...\n", i+1);
        cpu_time_used = ((float) (end - start)) / CLOCKS_PER_SEC;
        total_time += cpu_time_used;

        smCleanup(res);
    }
    
    float avg_time = total_time / num_runs;
    printf("\naverage execution time for matmul over %d runs: %f seconds\n\n", num_runs, avg_time);

    smCleanup(a);
    smCleanup(b);
    return 0;
}
