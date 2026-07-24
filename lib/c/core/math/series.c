/**
 * @file series.c
 * @brief Reducing a run of samples to the few numbers a chart draws from.
 */
#include "math/series.h"

int series_range(const uint8_t *samples, int count, uint8_t no_sample,
                 int *lo, int *hi, int *last)
{
    int lo_val = 0;
    int hi_val = 0;
    int last_val = 0;
    int valid = 0;

    for (int i = 0; i < count; i++)
    {
        int value = samples[i];
        if (value == no_sample)
        {
            continue;
        }

        if (valid == 0 || value < lo_val)
        {
            lo_val = value;
        }
        if (valid == 0 || value > hi_val)
        {
            hi_val = value;
        }
        last_val = value;   // slots run oldest to newest, so the last real one is the most recent
        valid++;
    }

    if (valid > 0)
    {
        *lo = lo_val;
        *hi = hi_val;
        *last = last_val;
    }
    return valid;
}

int series_max_u16(const uint16_t *values, int count)
{
    int hi = 0;
    for (int i = 0; i < count; i++)
    {
        if (values[i] > hi)
        {
            hi = values[i];
        }
    }
    return hi;
}
