#include <stdint.h>
#include <stdio.h>
#include "encode.h"

static int expect_delta(const char *name, uint16_t now, uint16_t last, int16_t want)
{
    int16_t got = Encode_Calc_Delta(now, last);

    if (got != want) {
        printf("%s: want %d, got %d\n", name, want, got);
        return 1;
    }

    return 0;
}

int main(void)
{
    int failed = 0;

    failed += expect_delta("normal forward", 130U, 100U, 30);
    failed += expect_delta("wrap forward", 1U, 65534U, 3);
    failed += expect_delta("wrap reverse", 65535U, 2U, -3);

    return failed;
}
