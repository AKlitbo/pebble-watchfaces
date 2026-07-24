/**
 * @file moon.c
 * @brief Pure moon-phase math, host-testable off-device
 */
#include "clock/moon.h"

int32_t moon_age_sec(time_t utc)
{
    // the offset from the epoch fits 32 bits until 2068 which outlives the watch.
    // staying in 32 bits keeps the 64 bit division helpers out of the binary
    int32_t age = (int32_t)(utc - MOON_EPOCH_UTC) % MOON_SYNODIC_SEC;
    if (age < 0)
    {
        age += MOON_SYNODIC_SEC;
    }

    return age;
}

int moon_glyph_index(time_t utc, int count)
{
    if (count <= 0)
    {
        return 0;
    }

    // round the age to the nearest of count evenly spaced glyphs around the ring.
    // age times count tops out around 71 million for the 28 glyph ring so int32 holds it
    int32_t age = moon_age_sec(utc);
    int idx = (int)(((age * count) + MOON_SYNODIC_SEC / 2) / MOON_SYNODIC_SEC);
    return idx % count;
}

int moon_illumination_pct(time_t utc)
{
    int32_t age = moon_age_sec(utc);
    int32_t half = MOON_SYNODIC_SEC / 2;

    // a triangle from new to full and back. not the true cosine curve, but close enough
    // for a percent readout and it stays pure integer math
    int32_t lit = age <= half ? (age * 100) / half
                              : ((MOON_SYNODIC_SEC - age) * 100) / half;
    return (int)lit;
}

int moon_days_to_phase(time_t utc, bool to_full)
{
    int32_t target = to_full ? MOON_SYNODIC_SEC / 2 : MOON_SYNODIC_SEC;
    int32_t until = target - moon_age_sec(utc);
    if (until < 0)
    {
        // just past the target so the next one is almost a whole cycle away
        until += MOON_SYNODIC_SEC;
    }

    // whole days rounded to the nearest so half a day out still reads right
    return (int)((until + 43200) / 86400);
}

const char *moon_phase_name(time_t utc)
{
    static const char *names[8] = {
        "NEW", "WAX CRES", "1ST QTR", "WAX GIB",
        "FULL", "WAN GIB", "3RD QTR", "WAN CRES",
    };

    return names[moon_glyph_index(utc, 8)];
}
