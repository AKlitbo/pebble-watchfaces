/**
 * @file ridgeline.c
 * @brief The Ridgeline ground table: the land under the shared sky, by day and by night, for every
 * theme.
 *
 * @ingroup watchface-sketchbook
 */
#include "theme/ridgeline.h"

#include <stddef.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// the engine reads the readouts through a GroundReadouts pointer, so they have to lead the row
_Static_assert(offsetof(RidgelineGround, readouts) == 0, "the readouts lead the ground row");

// by day the clock goes light against the land it sits on
static const RidgelineGround s_day[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        .readouts = {.text = GColorBlack, .dim = GColorDarkGray},
        .land_far = GColorLightGray,    .land     = GColorWhite,
    },
    [THEME_DAYBREAK] = {
        .readouts = {.text = GColorWhite, .dim = GColorMintGreen},
        .land_far = GColorJaegerGreen,  .land     = GColorDarkGreen,
    },
    [THEME_ALPENGLOW] = {
        .readouts = {.text = GColorRajah, .dim = GColorMelon},
        .land_far = GColorRoseVale,     .land     = GColorBulgarianRose,
    },
    [THEME_BLUEPRINT] = {
        .readouts = {.text = GColorWhite, .dim = GColorBabyBlueEyes},
        .land_far = GColorCobaltBlue,   .land     = GColorOxfordBlue,
    },
    [THEME_FOREST] = {
        .readouts = {.text = GColorSpringBud, .dim = GColorInchworm},
        .land_far = GColorArmyGreen,    .land     = GColorMidnightGreen,
    },
    [THEME_MONO] = {
        .readouts = {.text = GColorWhite, .dim = GColorWhite},
        .land_far = GColorLightGray,    .land     = GColorDarkGray,
    },
    [THEME_CYBERPUNK] = {
        // a city that never switches its signage off, so the readouts stay on neon
        .readouts = {.text = GColorCyan, .dim = GColorMagenta},
        .land_far = GColorImperialPurple, .land     = GColorBlack,
    },
    [THEME_NEO_TOKYO] = {
        // slate ridges under the haze
        .readouts = {.text = GColorWhite, .dim = GColorSunsetOrange},
        .land_far = GColorCadetBlue,    .land     = GColorOxfordBlue,
    },
};

// after dark the land goes dark enough for a light clock to sit straight on it
static const RidgelineGround s_night[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        .readouts = {.text = GColorWhite, .dim = GColorLightGray},
        .land_far = GColorOxfordBlue,   .land     = GColorBlack,
    },
    [THEME_DAYBREAK] = {
        .readouts = {.text = GColorWhite, .dim = GColorBabyBlueEyes},
        .land_far = GColorDukeBlue,     .land     = GColorBlack,
    },
    [THEME_ALPENGLOW] = {
        .readouts = {.text = GColorMelon, .dim = GColorRichBrilliantLavender},
        .land_far = GColorImperialPurple, .land     = GColorBlack,
    },
    [THEME_BLUEPRINT] = {
        .readouts = {.text = GColorWhite, .dim = GColorPictonBlue},
        .land_far = GColorOxfordBlue,   .land     = GColorBlack,
    },
    [THEME_FOREST] = {
        .readouts = {.text = GColorSpringBud, .dim = GColorMayGreen},
        .land_far = GColorArmyGreen,    .land     = GColorBlack,
    },
    [THEME_MONO] = {
        .readouts = {.text = GColorWhite, .dim = GColorLightGray},
        .land_far = GColorDarkGray,     .land     = GColorBlack,
    },
    [THEME_CYBERPUNK] = {
        // the clock in cyan, so it splits the screen with the magenta linework
        .readouts = {.text = GColorCyan, .dim = GColorMagenta},
        .land_far = GColorJazzberryJam, .land     = GColorBlack,
    },
    [THEME_NEO_TOKYO] = {
        // red neon bleeding up off the ridges, and it keeps white for the clock
        .readouts = {.text = GColorWhite, .dim = GColorSunsetOrange},
        .land_far = GColorBulgarianRose, .land     = GColorBlack,
    },
};

const void *ridgeline_ground_for_theme(uint8_t theme, bool night)
{
    if (theme >= THEME_COUNT)
    {
        theme = THEME_SKETCHBOOK;
    }

    return night ? &s_night[theme] : &s_day[theme];
}

/** @} */
