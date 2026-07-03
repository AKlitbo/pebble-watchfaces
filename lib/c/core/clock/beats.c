/**
 * @file beats.c
 * @brief Pure swatch .beats math, host-testable off-device
 */
#include "clock/beats.h"

int beats_from_ms(int64_t ms_into_bmt_day)
{
    if (ms_into_bmt_day < 0)
    {
        ms_into_bmt_day = 0;
    }

    return (int)((ms_into_bmt_day % MS_PER_DAY) / MS_PER_BEAT);
}

uint32_t ms_until_next_beat(int64_t ms_into_bmt_day)
{
    if (ms_into_bmt_day < 0)
    {
        ms_into_bmt_day = 0;
    }

    // ms position within the current beat
    int64_t into_beat = ms_into_bmt_day % MS_PER_BEAT;

    // result runs 1..MS_PER_BEAT and never hits 0
    // right on a boundary we report a full beat to the next flip
    return (uint32_t)(MS_PER_BEAT - into_beat);
}
