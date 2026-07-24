/**
 * @file solar.c
 * @brief How the day's light is going, worked out from three plain clock readings.
 */
#include "clock/solar.h"

// a day's worth of minutes, for wrapping the night span across midnight
#define MINUTES_PER_DAY 1440

// whether all three readings are real, so the maths below is not run on a missing time
static bool have_all(int rise, int set, int now)
{
    return rise >= 0 && set >= 0 && now >= 0;
}

int solar_day_progress(int rise, int set, int now)
{
    if (!have_all(rise, set, now) || now < rise || now > set)
    {
        return -1;
    }

    int span = set - rise;
    if (span <= 0)
    {
        return -1;
    }
    return ((now - rise) * 100) / span;
}

int solar_night_progress(int rise, int set, int now)
{
    if (!have_all(rise, set, now) || (now <= set && now >= rise))
    {
        return -1;
    }

    int span = rise + MINUTES_PER_DAY - set;   // sunset to tomorrow's sunrise
    if (span <= 0)
    {
        return -1;
    }
    int elapsed = (now > set) ? now - set : now + MINUTES_PER_DAY - set;
    return (elapsed * 100) / span;
}

int solar_next_event(int rise, int set, int now, bool *is_sunrise)
{
    if (!have_all(rise, set, now))
    {
        return -1;
    }

    if (now < rise)
    {
        *is_sunrise = true;
        return rise - now;
    }
    if (now < set)
    {
        *is_sunrise = false;
        return set - now;
    }
    *is_sunrise = true;
    return rise + MINUTES_PER_DAY - now;
}
