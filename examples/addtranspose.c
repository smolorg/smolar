#include <stdio.h>
#include "../smolar.h"

int main()
{
    int shape[] = {3, 1};
    int axis[] = {1, 0};

    Array *a = smReshapeNew(smArange(1, 4, 1), shape, 2);
    Array *b = smTransposeNew(
        smReshapeNew(smArange(1, 4, 1), shape, 2),
        axis
    );

    Array *res = smAdd(a, b);

    printf("Array a:\n");
    smShow(a);
    printf("Array b (a.T):\n");
    smShow(b);

    printf("Array a + b:\n");
    smShow(res);

    return 0;
}