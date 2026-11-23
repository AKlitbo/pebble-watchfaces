/**
 * @file skyline.c
 * @brief The Skyline ground table: the city, the bridge, and the river under the shared sky, by day
 * and by night, for every theme.
 *
 * @ingroup watchface-sketchbook
 */
#include "theme/skyline.h"

#include <stddef.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// the engine reads the readouts through a GroundReadouts pointer, so they have to lead the row
_Static_assert(offsetof(SkylineGround, readouts) == 0, "the readouts lead the ground row");

// by day the clock reads against the water it sits on. the lamps are mostly out, so `lamp` is
// carrying only the handful of windows that stay lit whatever the hour
static const SkylineGround s_day[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        // pen on paper: the river is white too, and only the reflections say it is there
        .readouts = {.text = GColorBlack, .dim = GColorDarkGray},
        .city   = GColorLightGray,      .bridge = GColorDarkGray,
        .water  = GColorWhite,          .lamp   = GColorBlack,
    },
    [THEME_DAYBREAK] = {
        .readouts = {.text = GColorWhite, .dim = GColorBabyBlueEyes},
        .city   = GColorCadetBlue,      .bridge = GColorBulgarianRose,
        .water  = GColorBlueMoon,       .lamp   = GColorIcterine,
    },
    [THEME_ALPENGLOW] = {
        .readouts = {.text = GColorMelon, .dim = GColorRichBrilliantLavender},
        .city   = GColorRoseVale,       .bridge = GColorBulgarianRose,
        .water  = GColorImperialPurple, .lamp   = GColorIcterine,
    },
    [THEME_BLUEPRINT] = {
        // blue paper, so the crossing is drawn rather than coloured in
        .readouts = {.text = GColorWhite, .dim = GColorBabyBlueEyes},
        .city   = GColorDukeBlue,       .bridge = GColorOxfordBlue,
        .water  = GColorCobaltBlue,     .lamp   = GColorBabyBlueEyes,
    },
    [THEME_FOREST] = {
        .readouts = {.text = GColorSpringBud, .dim = GColorInchworm},
        .city   = GColorArmyGreen,      .bridge = GColorMidnightGreen,
        .water  = GColorDarkGreen,      .lamp   = GColorIcterine,
    },
    [THEME_MONO] = {
        .readouts = {.text = GColorWhite, .dim = GColorWhite},
        .city   = GColorLightGray,      .bridge = GColorBlack,
        .water  = GColorDarkGray,       .lamp   = GColorWhite,
    },
    [THEME_CYBERPUNK] = {
        // a river that never switches its signage off, so the readouts stay on neon
        .readouts = {.text = GColorCyan, .dim = GColorMagenta},
        .city   = GColorImperialPurple, .bridge = GColorBlack,
        .water  = GColorJazzberryJam,   .lamp   = GColorCyan,
    },
    [THEME_NEO_TOKYO] = {
        // a slate river under the haze
        .readouts = {.text = GColorWhite, .dim = GColorSunsetOrange},
        .city   = GColorCadetBlue,      .bridge = GColorOxfordBlue,
        .water  = GColorLiberty,        .lamp   = GColorSunsetOrange,
    },
};

// after dark the water goes dark with the sky, which is the whole point: a dark river is what
// makes a string of lamps and their reflections the brightest thing on the screen. the bridge
// itself goes almost to silhouette so the lights carry the picture
static const SkylineGround s_night[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        .readouts = {.text = GColorWhite, .dim = GColorLightGray},
        .city   = GColorOxfordBlue,     .bridge = GColorBlack,
        .water  = GColorBlack,          .lamp   = GColorIcterine,
    },
    [THEME_DAYBREAK] = {
        .readouts = {.text = GColorWhite, .dim = GColorBabyBlueEyes},
        .city   = GColorDukeBlue,       .bridge = GColorBlack,
        .water  = GColorOxfordBlue,     .lamp   = GColorRajah,
    },
    [THEME_ALPENGLOW] = {
        .readouts = {.text = GColorMelon, .dim = GColorRichBrilliantLavender},
        .city   = GColorImperialPurple, .bridge = GColorBlack,
        .water  = GColorBlack,          .lamp   = GColorIcterine,
    },
    [THEME_BLUEPRINT] = {
        .readouts = {.text = GColorWhite, .dim = GColorPictonBlue},
        .city   = GColorOxfordBlue,     .bridge = GColorBlack,
        .water  = GColorOxfordBlue,     .lamp   = GColorPictonBlue,
    },
    [THEME_FOREST] = {
        .readouts = {.text = GColorSpringBud, .dim = GColorMayGreen},
        .city   = GColorMidnightGreen,  .bridge = GColorBlack,
        .water  = GColorBlack,          .lamp   = GColorIcterine,
    },
    [THEME_MONO] = {
        .readouts = {.text = GColorWhite, .dim = GColorLightGray},
        .city   = GColorDarkGray,       .bridge = GColorBlack,
        .water  = GColorBlack,          .lamp   = GColorWhite,
    },
    [THEME_CYBERPUNK] = {
        // a black river, with every lamp and every reflection burning cyan
        .readouts = {.text = GColorCyan, .dim = GColorMagenta},
        .city   = GColorJazzberryJam,   .bridge = GColorBlack,
        .water  = GColorBlack,          .lamp   = GColorCyan,
    },
    [THEME_NEO_TOKYO] = {
        // red neon bleeding off the far bank, and it keeps white for the clock
        .readouts = {.text = GColorWhite, .dim = GColorSunsetOrange},
        .city   = GColorBulgarianRose,  .bridge = GColorBlack,
        .water  = GColorBlack,          .lamp   = GColorSunsetOrange,
    },
};

const void *skyline_ground_for_theme(uint8_t theme, bool night)
{
    if (theme >= THEME_COUNT)
    {
        theme = THEME_SKETCHBOOK;
    }

    return night ? &s_night[theme] : &s_day[theme];
}

/** @} */
