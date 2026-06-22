/**
 * @file units.c
 * @brief SDK-facing units facade: reads the clock for .beats, forwards the rest to
 * the pure lib cores (clock/beats, units/distance)
 */
#include "units.h"

#include "beats.h"
#include "distance.h"

/**
 * @brief Calculate ms since Biel Mean Time (UTC+1) midnight.
 *
 * Has sub-second precision. .beats boundaries fall mid-second, so we work
 * in ms. Flooring whole seconds reads one beat behind right when the
 * refresh timer fires.
 *
 * @return Milliseconds into the current BMT day.
 */
static int64_t ms_into_bmt_day(void)
{
    time_t now;
    uint16_t ms;
    time_ms(&now, &ms);  // seconds + the ms within that second

    struct tm *utc = gmtime(&now);
    long secs = utc->tm_hour * 3600 + utc->tm_min * 60 + utc->tm_sec;

    return (((int64_t)((secs + 3600) % 86400)) * 1000LL) + ms;
}

int units_swatch_beats(void)
{
    return beats_from_ms(ms_into_bmt_day());
}

uint32_t units_ms_until_next_beat(void)
{
    return ms_until_next_beat(ms_into_bmt_day());
}

void units_format_distance(char *buffer, size_t size, int meters, bool miles)
{
    distance_format(buffer, size, meters, miles);
}
