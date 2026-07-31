/**
 * @file tide.c
 * @brief The rise and fall of a semidiurnal tide, from a plain running minute count.
 */
#include "clock/tide.h"

// where in the cycle a minute count lands, 0 to TIDE_PERIOD_MIN - 1. C truncates a negative
// division towards zero, which leaves a negative remainder, so it is pulled back up by hand
static int32_t phase(int32_t minutes)
{
    int32_t within = minutes % TIDE_PERIOD_MIN;

    return within < 0 ? within + TIDE_PERIOD_MIN : within;
}

int tide_level(int32_t minutes)
{
    // the cycle as a triangle first: up over the first half of the period, back down over the
    // second
    int32_t pos = (phase(minutes) * 200) / TIDE_PERIOD_MIN;  // 0..199
    int32_t tri = pos <= 100 ? pos : 200 - pos;              // 0..100

    // then eased with a smoothstep, which flattens both turns and steepens the middle. it is
    // exact at 0, 50 and 100, so dead low and high water really do reach the ends
    return (int)((tri * tri * (300 - 2 * tri)) / 10000);
}

bool tide_rising(int32_t minutes)
{
    return phase(minutes) * 2 < TIDE_PERIOD_MIN;
}
