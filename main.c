#include <stdio.h>
#include <time.h>

#include "smolar.h"

int main()
{
    const int shapea[] = {2, 1, 3};
    const int shapeb[] = {2, 2, 3};

    Array *a = smReshapeNew(smArange(1.0, 7.0, 1.0), shapea, 3);
    Array *b = smReshapeNew(smArange(1.0, 13.0, 1.0), shapeb, 3);

    Array *c = smAdd(a, b);

    printf("Array a:\n");
    smShow(a);
    printf("Array b:\n");
    smShow(b);
    printf("\nArray c:\n");
    smShow(c);
    return 0;
}
