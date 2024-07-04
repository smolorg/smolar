#include <stdio.h>
#include "../smolar.h"

int main()
{
    int shape[] = {3, 1, 2, 1};

    Array *a = smReshapeNew(smArange(1, 7, 1), shape, 4);
    Array *res = smSqueeze(smSqueeze(a, -1), 1);

    __printArrayInternals__(a, a->shape);
    __printArrayInternals__(res, res->shape);
    printf("Array:\n");
    smShow(a);
    printf("Result:\n");
    smShow(res);

    smCleanup(a);
    smCleanup(res);
    return 0;
}