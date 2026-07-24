/**
 * @file beats.c
 * @brief Pure swatch .beats math, host-testable off-device
 */
#include "clock/beats.h"

int beats_from_ms(int32_t ms_into_bmt_day)
{
    if (ms_into_bmt_day < 0)
    {
        ms_into_bmt_day = 0;
    }

    return (int)((ms_into_bmt_day % MS_PER_DAY) / MS_PER_BEAT);
}

uint32_t ms_until_next_beat(int32_t ms_into_bmt_day)
{
    if (ms_into_bmt_day < 0)
    {
        ms_into_bmt_day = 0;
    }

    // ms position within the current beat
    int32_t into_beat = ms_into_bmt_day % MS_PER_BEAT;

    // result runs 1..MS_PER_BEAT and never hits 0
    // right on a boundary we report a full beat to the next flip
    return (uint32_t)(MS_PER_BEAT - into_beat);
}

int32_t beats_ms_from_hms(int hour, int minute, int second, uint16_t ms)
{
    int32_t secs = hour * SECS_PER_HOUR + minute * SECS_PER_MIN + second;

    // caps just under a day of ms (86.4 million) so int32 holds it exactly
    return (((secs + BMT_UTC_OFFSET_S) % SECS_PER_DAY) * MS_PER_SEC) + ms;
}
