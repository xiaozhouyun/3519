#include <stdint.h>
#include <stdio.h>
#include "drv8873.h"

static int expect_i16(const char *name, int16_t got, int16_t want)
{
    if (got != want) {
        printf("%s: want %d, got %d\n", name, want, got);
        return 1;
    }

    return 0;
}

static int expect_u16(const char *name, uint16_t got, uint16_t want)
{
    if (got != want) {
        printf("%s: want %u, got %u\n", name, want, got);
        return 1;
    }

    return 0;
}

static int expect_u32(const char *name, uint32_t got, uint32_t want)
{
    if (got != want) {
        printf("%s: want %lu, got %lu\n", name, (unsigned long)want, (unsigned long)got);
        return 1;
    }

    return 0;
}

int main(void)
{
    int failed = 0;

    // 1. 超限测试 (1200 限幅至 1000, 满速 compare=0, 方向 FORWARD=1)
    DRV8873_Control_t c1 = DRV8873_Speed_To_Control(1200);
    failed += expect_i16("limits high dir", (int16_t)c1.dir, (int16_t)DRV8873_DIR_FORWARD);
    failed += expect_u16("limits high compare", c1.compare, 0U);

    // 2. 负向半速测试 (-500 绝对值 500, duty 500, compare 1000-500=500, 方向 REVERSE=0)
    DRV8873_Control_t c2 = DRV8873_Speed_To_Control(-500);
    failed += expect_i16("half speed rev dir", (int16_t)c2.dir, (int16_t)DRV8873_DIR_REVERSE);
    failed += expect_u16("half speed rev compare", c2.compare, 500U);

    // 3. 零速测试 (0 速度, compare 1000, 方向 FORWARD=1)
    DRV8873_Control_t c3 = DRV8873_Speed_To_Control(0);
    failed += expect_u16("zero speed compare", c3.compare, 1000U);

    // 4. 电流采样毫安换算测试
    failed += expect_u32("full scale current mA", DRV8873_Adc_To_Current_mA(4095U), 1210U);

    return failed;
}


