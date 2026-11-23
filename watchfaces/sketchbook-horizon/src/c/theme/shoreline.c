/**
 * @file shoreline.c
 * @brief The Shoreline ground table: the sea and the sand under the shared sky, by day and by night,
 * for every theme.
 *
 * @ingroup watchface-sketchbook
 */
#include "theme/shoreline.h"

#include <stddef.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// the engine reads the readouts through a GroundReadouts pointer, so they have to lead the row
_Static_assert(offsetof(ShorelineGround, readouts) == 0, "the readouts lead the ground row");

// by day the clock reads against the sand it sits on. the bands run far water, near water, wet
// sand, dry sand from the horizon down, and neighbouring bands are kept apart or the waterline
// stops reading
static const ShorelineGround s_day[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        // pen on paper: the water is white too, and only the horizon, the wave marks and the
        // surf line say where the sea is
        .readouts = {.text = GColorBlack, .dim = GColorDarkGray},
        .sea_far = GColorLightGray,     .sea     = GColorWhite,
        .sand    = GColorWhite,         .wet     = GColorLightGray,
        .foam    = GColorBlack,
    },
    [THEME_DAYBREAK] = {
        .readouts = {.text = GColorBlack, .dim = GColorDarkGreen},
        .sea_far = GColorJaegerGreen,   .sea     = GColorMediumAquamarine,
        .sand    = GColorPastelYellow,  .wet     = GColorBrass,
        .foam    = GColorWhite,
    },
    [THEME_ALPENGLOW] = {
        .readouts = {.text = GColorBulgarianRose, .dim = GColorImperialPurple},
        .sea_far = GColorImperialPurple, .sea     = GColorBulgarianRose,
        .sand    = GColorMelon,         .wet     = GColorRoseVale,
        .foam    = GColorWhite,
    },
    [THEME_BLUEPRINT] = {
        // blue paper, so the beach is drawn rather than coloured in
        .readouts = {.text = GColorWhite, .dim = GColorBabyBlueEyes},
        .sea_far = GColorDukeBlue,      .sea     = GColorCobaltBlue,
        .sand    = GColorOxfordBlue,    .wet     = GColorDukeBlue,
        .foam    = GColorWhite,
    },
    [THEME_FOREST] = {
        // the water is teal rather than green here. the boat's hull is solid ink, and a
        // dark green sea would swallow it whole
        .readouts = {.text = GColorSpringBud, .dim = GColorInchworm},
        .sea_far = GColorMidnightGreen, .sea     = GColorTiffanyBlue,
        .sand    = GColorMidnightGreen, .wet     = GColorArmyGreen,
        .foam    = GColorWhite,
    },
    [THEME_MONO] = {
        .readouts = {.text = GColorWhite, .dim = GColorWhite},
        .sea_far = GColorWhite,         .sea     = GColorLightGray,
        .sand    = GColorDarkGray,      .wet     = GColorBlack,
        .foam    = GColorWhite,
    },
    [THEME_CYBERPUNK] = {
        // a seafront that never switches its signage off, so the readouts stay on neon
        .readouts = {.text = GColorCyan, .dim = GColorMagenta},
        .sea_far = GColorImperialPurple, .sea     = GColorJazzberryJam,
        .sand    = GColorBlack,         .wet     = GColorImperialPurple,
        .foam    = GColorMagenta,
    },
    [THEME_NEO_TOKYO] = {
        // slate water and a dark shore under the haze
        .readouts = {.text = GColorWhite, .dim = GColorSunsetOrange},
        .sea_far = GColorCadetBlue,     .sea     = GColorLiberty,
        .sand    = GColorOxfordBlue,    .wet     = GColorBlack,
        .foam    = GColorWhite,
    },
};

// after dark the sand goes dark enough for a light clock to sit straight on it. the bands
// alternate rather than all going black, which is the only thing keeping the water apart from
// the beach
static const ShorelineGround s_night[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        .readouts = {.text = GColorWhite, .dim = GColorLightGray},
        .sea_far = GColorOxfordBlue,    .sea     = GColorBlack,
        .sand    = GColorBlack,         .wet     = GColorOxfordBlue,
        .foam    = GColorWhite,
    },
    [THEME_DAYBREAK] = {
        .readouts = {.text = GColorWhite, .dim = GColorBabyBlueEyes},
        .sea_far = GColorDukeBlue,      .sea     = GColorOxfordBlue,
        .sand    = GColorBlack,         .wet     = GColorDukeBlue,
        .foam    = GColorBabyBlueEyes,
    },
    [THEME_ALPENGLOW] = {
        .readouts = {.text = GColorMelon, .dim = GColorRichBrilliantLavender},
        .sea_far = GColorImperialPurple, .sea     = GColorBlack,
        .sand    = GColorBlack,         .wet     = GColorImperialPurple,
        .foam    = GColorRichBrilliantLavender,
    },
    [THEME_BLUEPRINT] = {
        .readouts = {.text = GColorWhite, .dim = GColorPictonBlue},
        .sea_far = GColorOxfordBlue,    .sea     = GColorBlack,
        .sand    = GColorBlack,         .wet     = GColorOxfordBlue,
        .foam    = GColorPictonBlue,
    },
    [THEME_FOREST] = {
        .readouts = {.text = GColorSpringBud, .dim = GColorMayGreen},
        .sea_far = GColorMidnightGreen, .sea     = GColorBlack,
        .sand    = GColorBlack,         .wet     = GColorArmyGreen,
        .foam    = GColorMintGreen,
    },
    [THEME_MONO] = {
        .readouts = {.text = GColorWhite, .dim = GColorLightGray},
        .sea_far = GColorDarkGray,      .sea     = GColorBlack,
        .sand    = GColorBlack,         .wet     = GColorDarkGray,
        .foam    = GColorWhite,
    },
    [THEME_CYBERPUNK] = {
        // magenta surf over black water, with the clock in cyan so the two neons split the
        // screen between them
        .readouts = {.text = GColorCyan, .dim = GColorMagenta},
        .sea_far = GColorJazzberryJam,  .sea     = GColorBlack,
        .sand    = GColorBlack,         .wet     = GColorImperialPurple,
        .foam    = GColorMagenta,
    },
    [THEME_NEO_TOKYO] = {
        // red neon bleeding up off the front, and it keeps white for the clock
        .readouts = {.text = GColorWhite, .dim = GColorSunsetOrange},
        .sea_far = GColorBulgarianRose, .sea     = GColorBlack,
        .sand    = GColorBlack,         .wet     = GColorBulgarianRose,
        .foam    = GColorSunsetOrange,
    },
};

const void *shoreline_ground_for_theme(uint8_t theme, bool night)
{
    if (theme >= THEME_COUNT)
    {
        theme = THEME_SKETCHBOOK;
    }

    return night ? &s_night[theme] : &s_day[theme];
}

/** @} */
