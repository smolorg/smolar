#include <stdio.h>
#include "../smolar.h"

int main()
{
    int shape[] = {10};

    Array *a = smArange(1, 11, 1);
    Array *b = smArange(1, 11, 1);

    Array *res = smDot(a, b);

    smShow(a);
    smShow(b);
    smShow(res);

    smPrintInfo(res);

    smCleanup(a);
    smCleanup(b);
    smCleanup(res);

    return 0;
}