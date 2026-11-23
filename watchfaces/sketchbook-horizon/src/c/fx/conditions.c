/**
 * @file conditions.c
 * @brief The condition table: what each token does to the sky.
 *
 * @ingroup watchface-sketchbook
 */
#include "fx/conditions.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// one row per condition. every scene draws the same sky for a given condition, which is the whole
// reason this is here rather than in each of them
#define SKY_CLEAR {.clouds = CLOUDS_NONE,     .celestial = CEL_FULL,   .precip = PRECIP_NONE}
#define SKY_PCLDY {.clouds = CLOUDS_DRIFTING, .celestial = CEL_PEEK,   .precip = PRECIP_NONE}
#define SKY_CLDY  {.clouds = CLOUDS_OVERCAST, .celestial = CEL_HIDDEN, .precip = PRECIP_NONE}
#define SKY_FOGGY {.clouds = CLOUDS_DRIFTING, .celestial = CEL_HIDDEN, .precip = PRECIP_NONE, .wash = true}
#define SKY_DRZL  {.clouds = CLOUDS_DRIFTING, .celestial = CEL_PEEK,   .precip = PRECIP_DROPS,  .drops = 16}
#define SKY_FZDZ  {.clouds = CLOUDS_OVERCAST, .celestial = CEL_HIDDEN, .precip = PRECIP_MIXED,  .drops = 18}
#define SKY_RAIN  {.clouds = CLOUDS_OVERCAST, .celestial = CEL_HIDDEN, .precip = PRECIP_DASHES, .drops = 28}
#define SKY_FZRN  {.clouds = CLOUDS_OVERCAST, .celestial = CEL_HIDDEN, .precip = PRECIP_MIXED,  .drops = 24}
#define SKY_SNOW  {.clouds = CLOUDS_OVERCAST, .celestial = CEL_HIDDEN, .precip = PRECIP_FLAKES, .drops = 22}
// a shower is a passing cloud, so the sun stays in shot behind it
#define SKY_SHWR  {.clouds = CLOUDS_DRIFTING, .celestial = CEL_PEEK,   .precip = PRECIP_DASHES, .drops = 20}
#define SKY_SNSH  {.clouds = CLOUDS_DRIFTING, .celestial = CEL_PEEK,   .precip = PRECIP_FLAKES, .drops = 18}
#define SKY_STRM  {.clouds = CLOUDS_OVERCAST, .celestial = CEL_HIDDEN, .precip = PRECIP_DASHES, .drops = 30, .bolt = true}

#define SKY_ROW(name) [WX_COND_##name] = SKY_##name, ///< One condition's row, from its SKY_ define

/**
 * @brief The sky table, indexed by WxCond.
 *
 * It is built from the framework's list of conditions, so a condition added anywhere in that list
 * stops the build on a SKY_ row this file has not got.
 */
static const SketchbookSky s_skies[WX_COND_COUNT] = {WX_COND_EACH(SKY_ROW)};

WxCond sketchbook_cond_parse(const char *condition)
{
    // the framework already drops "_NIGHT" and finds the row. all that is added here is reading a
    // token it does not know, or none at all, as open sky
    WxCond cond = wx_cond_parse(condition, NULL);
    return cond < WX_COND_COUNT ? cond : WX_COND_CLEAR;
}

SketchbookSky sketchbook_sky_recipe(WxCond cond)
{
    if (cond >= WX_COND_COUNT)
    {
        return s_skies[WX_COND_CLEAR];
    }

    return s_skies[cond];
}

bool sketchbook_cond_snows(WxCond cond)
{
    return cond == WX_COND_SNOW || cond == WX_COND_SNSH;
}

/** @} */
