#ifndef FAST_MATH_H
#define FAST_MATH_H

#if defined(__GNUC__) || defined(__clang__)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#define FORCE_INLINE inline
#endif

FORCE_INLINE static float fast_fabsf(float x)
{
    return (x < 0.0f) ? -x : x;
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

/* 快速 atan2(y, x)，0/0 时按 0 返回，避免除零。 */
FORCE_INLINE static float fast_atan2f(float y, float x)
{
    const float PI_2 = 1.5707963268f;
    float abs_y = fast_fabsf(y);
    float r;
    float angle;

    if ((x == 0.0f) && (y == 0.0f)) {
        return 0.0f;
    }

    if (x >= 0.0f) {
        r = (x - abs_y) / (x + abs_y);
        angle = PI_2 / 2.0f - r * PI_2 / 2.0f;
    } else {
        r = (x + abs_y) / (abs_y - x);
        angle = PI_2 - r * PI_2 / 2.0f;
    }

    return (y < 0.0f) ? -angle : angle;
}

#endif
