/**
 * @file astro.c
 * @brief Pure astronomy math, host-testable off-device
 */
#include "clock/astro.h"

int32_t astro_jd_centi(time_t utc)
{
    // JD = 2440587.5 + utc/86400. split the seconds into whole days and the leftover so the
    // multiply stays in 32 bits. the .5 is already baked into 244058750
    long q = (long)utc / 86400;
    long r = (long)utc % 86400;
    return (int32_t)(244058750L + q * 100 + (r * 100) / 86400);
}
