#include <stdio.h>
#include <time.h>

#include "../smolar.h"

int main()
{
    int shape_a[] = {5, 4, 2, 3};
    int shape_b[] = {5, 4, 3, 4};

    int num_runs = 5;
    clock_t start, end;
    float cpu_time_used, total_time = 0.0f;

    Array *a = smReshapeNew(smArange(1, (5*4*2*3) + 1, 1), shape_a, 4);
    Array *b = smReshapeNew(smArange(1, (5*4*3*4) + 1, 1), shape_b, 4);

    printf("\nA shape:\n");
    __printArrayInternals__(a, a->shape);
    printf("B shape:\n");
    __printArrayInternals__(b, b->shape);

    printf("benchmarking matmul...\n");
    for (int i = 0; i < num_runs; i++)
    {
        start = clock();
        Array *res = smMatMul(a, b);
        end = clock();

        printf("completed run %d...\n", i + 1);
        cpu_time_used = ((float)(end - start)) / CLOCKS_PER_SEC;
        total_time += cpu_time_used;

        smCleanup(res);
    }

    float avg_time = total_time / num_runs;
    printf("\naverage execution time for matmul over %d runs: %f seconds\n\n", num_runs, avg_time);

    smCleanup(a);
    smCleanup(b);
    return 0;
}
