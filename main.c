#include <stdio.h>
#include <time.h>

#include "smolar.h"

int main()
{
    const int shapea[] = {3, 1};
    const int shapeb[] = {2};

    Array *a = smReshapeNew(smArange(1.0, 4.0, 1.0), shapea, 2);
    Array *b = smReshapeNew(smArange(1.0, 3.0, 1.0), shapeb, 1);

    Array *c = smAdd(a, b);

    printf("Array a:\n");
    smShow(a);
    printf("Array b:\n");
    smShow(b);
    printf("\nArray c:\n");
    smShow(c);
    return 0;
}
