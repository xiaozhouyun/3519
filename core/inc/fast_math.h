#ifndef FAST_MATH_H
#define FAST_MATH_H

#include <stdint.h>

#if defined(__GNUC__) || defined(__clang__)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#define FORCE_INLINE inline
#endif

#define PI_F     3.1415926535f
#define PI2_F    (2.0f * PI_F)
#define PI_2_F   (0.5f * PI_F)

FORCE_INLINE static float fast_fabsf(float x)
{
    union {
        float f;
        uint32_t i;
    } u = { .f = x };

    u.i &= 0x7FFFFFFFU;
    return u.f;
}

/* 快速 sin(x)，适合 x 已经限制在 -pi 到 pi 附近的场景。 */
FORCE_INLINE static float fast_sinf(float x)
{
    const float B = 1.27324f;
    const float C = -0.405285f;
    const float P = 0.225f;

    float y = B * x + C * x * fast_fabsf(x);
    y = P * (y * fast_fabsf(y) - y) + y;
    return y;
}

/* 快速 开平方sqrt(x)，负数按 0 处理。 */
FORCE_INLINE static float fast_sqrtf(float x)
{
    if (x <= 0.0f) {
        return 0.0f;
    }

    const float half_x = 0.5f * x;
    union {
        float f;
        uint32_t i;
    } u = { .f = x };

    u.i = 0x5F3759DFU - (u.i >> 1);
    u.f = u.f * (1.5f - half_x * u.f * u.f);
    u.f = u.f * (1.5f - half_x * u.f * u.f);
    return x * u.f;
}

/* 快速 atan2(y, x)，0/0 时按 0 返回，避免除零。 */
FORCE_INLINE static float fast_atan2f(float y, float x)
{
    float abs_y = fast_fabsf(y);
    float r;
    float angle;

    if ((x == 0.0f) && (y == 0.0f)) {
        return 0.0f;
    }

    if (x >= 0.0f) {
        r = (x - abs_y) / (x + abs_y);
        angle = PI_2_F / 2.0f - r * PI_2_F / 2.0f;
    } else {
        r = (x + abs_y) / (abs_y - x);
        angle = PI_2_F - r * PI_2_F / 2.0f;
    }

    return (y < 0.0f) ? -angle : angle;
}

/* 快速 asin(x) = atan2(x, sqrt(1 - x²))，基于 fast_atan2f 实现 */
FORCE_INLINE static float fast_asinf(float x)
{
    /* 钳位到 [-1, 1] */
    if (x > 1.0f)  x = 1.0f;
    if (x < -1.0f) x = -1.0f;
    float denom = fast_sqrtf(1.0f - x * x);
    return fast_atan2f(x, denom);
}

#endif
