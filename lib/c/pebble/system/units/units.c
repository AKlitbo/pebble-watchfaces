/**
 * @file units.c
 * @brief SDK-facing units facade: reads the clock for .beats, forwards the rest to
 * the pure lib cores (clock/beats, units/distance)
 */
#include "system/units/units.h"

#include "clock/beats.h"
#include "units/distance.h"

// read the clock, break it apart, and hand the pieces to the maths. reading and breaking apart are
// the parts that need the SDK: gmtime and struct tm are pebble.h's here rather than time.h's, so
// they stay on this side of the fence and the sum lives next door in core
static int32_t ms_into_bmt_day(void)
{
    time_t now;
    uint16_t ms;
    time_ms(&now, &ms);  // seconds + the ms within that second

    struct tm *utc = gmtime(&now);
    if (!utc)
    {
        // unconvertible time_t: degrade to ms into a notional midnight rather than deref null
        return (int32_t)ms;
    }

    return beats_ms_from_hms(utc->tm_hour, utc->tm_min, utc->tm_sec, ms);
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

void units_format_distance_value(char *buffer, size_t size, int meters, bool miles)
{
    distance_format_value(buffer, size, meters, miles);
}

const char *units_distance_unit(bool miles)
{
    return distance_unit(miles);
}
