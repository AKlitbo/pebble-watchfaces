/**
 * @file grid_shim.c
 * @brief The clock and time zone shapes the shared panel bodies ask their face for.
 *
 * A panel body calls back into its face for the handful of things it cannot work out on its own.
 * The goals, units and week start come straight off this face's settings, so those live in
 * settings_schema.c beside the fields they read. What is left here is the pair this face genuinely
 * shapes differently: it carries one time zone rather than gridlock's two, and it writes the clock
 * through its own formatter.
 *
 * @ingroup watchface-sidereel
 */
#include "settings_schema.h"

#include "draw/text.h"

#include <stdlib.h>
#include <string.h>

/**
 * @addtogroup watchface-sidereel
 * @{
 */

int16_t gridlock_time_zone_offset_minutes(uint8_t index)
{
    // one zone here rather than gridlock's pair, so the index is ignored. the offset rides at the
    // front of the wire string and atoi stops at the comma
    (void)index;

    return (int16_t)atoi(sidereel_timezone_1());
}

const char *gridlock_time_zone_name(uint8_t index)
{
    (void)index;

    const char *comma = strchr(sidereel_timezone_1(), ',');

    // the caller cuts this at the next comma itself, so hand back the whole tail
    return comma ? comma + 1 : "";
}

bool gridlock_clock_is_24h(void)
{
    return !side_is_12h();
}

bool gridlock_format_clock(char *out, size_t n, int hour24, int minute)
{
    // keep the numbers in range so the formatted string can never overrun
    if (hour24 < 0 || hour24 > 23)
    {
        hour24 = 0;
    }

    if (minute < 0 || minute > 59)
    {
        minute = 0;
    }

    side_format_clock(out, n, hour24, minute);

    // true means the caller should put an AM or PM next to it
    return side_is_12h();
}

/** @} */
