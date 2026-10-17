/**
 * @file conditions.c
 * @brief The condition table: what each token does to the sky.
 *
 * @ingroup family-sketchbook
 */
#include "sketchbook/fx/conditions.h"

#include <string.h>

/**
 * @addtogroup family-sketchbook
 * @{
 */

/// the wire spelling of each row, indexed by SketchbookCond and matching it exactly
static const char *const s_tokens[SKETCHBOOK_COND_COUNT] = {
    "CLEAR", "PCLDY", "CLDY", "FOGGY", "DRZL", "FZDZ",
    "RAIN", "FZRN", "SNOW", "SHWR", "SNSH", "STRM",
};

// one row per condition, in the same order. every face in the family draws the same sky for a
// given condition, which is the whole reason this is here rather than in each of them
static const SketchbookSky s_skies[SKETCHBOOK_COND_COUNT] = {
    [SKETCHBOOK_COND_CLEAR] = {.clouds = CLOUDS_NONE,     .celestial = CEL_FULL,   .precip = PRECIP_NONE},
    [SKETCHBOOK_COND_PCLDY] = {.clouds = CLOUDS_DRIFTING, .celestial = CEL_PEEK,   .precip = PRECIP_NONE},
    [SKETCHBOOK_COND_CLDY]  = {.clouds = CLOUDS_OVERCAST, .celestial = CEL_HIDDEN, .precip = PRECIP_NONE},
    [SKETCHBOOK_COND_FOGGY] = {.clouds = CLOUDS_DRIFTING, .celestial = CEL_HIDDEN, .precip = PRECIP_NONE, .wash = true},
    [SKETCHBOOK_COND_DRZL]  = {.clouds = CLOUDS_DRIFTING, .celestial = CEL_PEEK,   .precip = PRECIP_DROPS,  .drops = 16},
    [SKETCHBOOK_COND_FZDZ]  = {.clouds = CLOUDS_OVERCAST, .celestial = CEL_HIDDEN, .precip = PRECIP_MIXED,  .drops = 18},
    [SKETCHBOOK_COND_RAIN]  = {.clouds = CLOUDS_OVERCAST, .celestial = CEL_HIDDEN, .precip = PRECIP_DASHES, .drops = 28},
    [SKETCHBOOK_COND_FZRN]  = {.clouds = CLOUDS_OVERCAST, .celestial = CEL_HIDDEN, .precip = PRECIP_MIXED,  .drops = 24},
    [SKETCHBOOK_COND_SNOW]  = {.clouds = CLOUDS_OVERCAST, .celestial = CEL_HIDDEN, .precip = PRECIP_FLAKES, .drops = 22},
    // a shower is a passing cloud, so the sun stays in shot behind it
    [SKETCHBOOK_COND_SHWR]  = {.clouds = CLOUDS_DRIFTING, .celestial = CEL_PEEK,   .precip = PRECIP_DASHES, .drops = 20},
    [SKETCHBOOK_COND_SNSH]  = {.clouds = CLOUDS_DRIFTING, .celestial = CEL_PEEK,   .precip = PRECIP_FLAKES, .drops = 18},
    [SKETCHBOOK_COND_STRM]  = {.clouds = CLOUDS_OVERCAST, .celestial = CEL_HIDDEN, .precip = PRECIP_DASHES, .drops = 30, .bolt = true},
};

SketchbookCond sketchbook_cond_parse(const char *condition)
{
    if (!condition || !condition[0])
    {
        return SKETCHBOOK_COND_CLEAR;
    }

    // "_NIGHT" rides on the token but says nothing about the sky, so compare up to it
    size_t len = strlen(condition);
    if (len > 6 && !strcmp(condition + len - 6, "_NIGHT"))
    {
        len -= 6;
    }

    for (int i = 0; i < SKETCHBOOK_COND_COUNT; i++)
    {
        if (strlen(s_tokens[i]) == len && !strncmp(s_tokens[i], condition, len))
        {
            return (SketchbookCond)i;
        }
    }

    return SKETCHBOOK_COND_CLEAR;
}

SketchbookSky sketchbook_sky_recipe(SketchbookCond cond)
{
    if (cond >= SKETCHBOOK_COND_COUNT)
    {
        return s_skies[SKETCHBOOK_COND_CLEAR];
    }

    return s_skies[cond];
}

/** @} */
