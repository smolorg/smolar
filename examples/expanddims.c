#include <stdio.h>
#include "../smolar.h"

int main()
{
    Array *a = smArange(1, 4, 1);
    Array *res = smExpandDims(smExpandDims(a, -1), 0);

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