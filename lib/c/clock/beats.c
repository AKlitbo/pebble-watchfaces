/**
 * @file beats.c
 * @brief pure swatch .beats math, host-testable off-device
 */
#include "beats.h"

// a .beats day is 1000 beats of 86400 ms each (86,400,000 ms total)
int beats_from_ms(int64_t ms_into_bmt_day)
{
    return (int)((ms_into_bmt_day % 86400000LL) / 86400LL);
}

uint32_t ms_until_next_beat(int64_t ms_into_bmt_day)
{
    // ms position within the current 86400 ms beat
    int64_t into_beat = ms_into_bmt_day % 86400LL;

    // 1..86400, never 0: on a boundary we report a full beat to the next flip
    return (uint32_t)(86400LL - into_beat);
}
