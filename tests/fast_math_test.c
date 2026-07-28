#include <math.h>
#include <stdio.h>
#include "fast_math.h"

static int expect_close(const char *name, float got, float want, float tolerance)
{
    float diff = got - want;

    if (diff < 0.0f) {
        diff = -diff;
    }

    if (diff > tolerance) {
        printf("%s: want %.6f, got %.6f\n", name, (double)want, (double)got);
        return 1;
    }

    return 0;
}

int main(void)
{
    int failed = 0;

    failed += expect_close("sin zero", fast_sinf(0.0f), 0.0f, 0.001f);
    failed += expect_close("sin half pi", fast_sinf(1.5707963268f), 1.0f, 0.02f);
    failed += expect_close("atan2 y axis", fast_atan2f(1.0f, 0.0f), 1.5707963268f, 0.02f);
    failed += expect_close("atan2 negative y", fast_atan2f(-1.0f, 0.0f), -1.5707963268f, 0.02f);
    failed += expect_close("fabs negative", fast_fabsf(-3.5f), 3.5f, 0.001f);

    return failed;
}
