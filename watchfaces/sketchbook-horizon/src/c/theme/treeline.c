/**
 * @file treeline.c
 * @brief The Treeline ground table: the firs, the cabin, and the clearing under the shared sky, by
 * day and by night, for every theme.
 *
 * @ingroup watchface-sketchbook
 */
#include "theme/treeline.h"

#include <stddef.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// the engine reads the readouts through a GroundReadouts pointer, so they have to lead the row
_Static_assert(offsetof(TreelineGround, readouts) == 0, "the readouts lead the ground row");

// by day the clock reads against the clearing it sits on. the window has no glow by day, so its
// daylight value is just the wall shade one step darker
static const TreelineGround s_day[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        // pen on paper, so the ink goes in the near trees and the far ones are left as
        // outlines. the other way round is truer to a fading distance and gives the front of
        // the wood nothing to read as, which on a stand of firs is most of the picture
        .readouts = {.text = GColorBlack, .dim = GColorDarkGray},
        .tree_far = GColorWhite,        .tree     = GColorLightGray,
        .ground   = GColorWhite,        .cabin    = GColorWhite,
        .roof     = GColorLightGray,    .glow     = GColorDarkGray,
        .smoke    = GColorLightGray,
    },
    [THEME_DAYBREAK] = {
        .readouts = {.text = GColorBlack, .dim = GColorDarkGreen},
        .tree_far = GColorJaegerGreen,  .tree     = GColorDarkGreen,
        .ground   = GColorMayGreen,     .cabin    = GColorWindsorTan,
        .roof     = GColorBulgarianRose, .glow     = GColorRajah,
        .smoke    = GColorWhite,
    },
    [THEME_ALPENGLOW] = {
        .readouts = {.text = GColorBulgarianRose, .dim = GColorImperialPurple},
        .tree_far = GColorBulgarianRose, .tree     = GColorImperialPurple,
        .ground   = GColorMelon,        .cabin    = GColorRoseVale,
        .roof     = GColorBulgarianRose, .glow     = GColorIcterine,
        .smoke    = GColorMelon,
    },
    [THEME_BLUEPRINT] = {
        // blue paper, so the forest is drawn rather than coloured in
        .readouts = {.text = GColorWhite, .dim = GColorBabyBlueEyes},
        .tree_far = GColorDukeBlue,     .tree     = GColorOxfordBlue,
        .ground   = GColorCobaltBlue,   .cabin    = GColorDukeBlue,
        .roof     = GColorOxfordBlue,   .glow     = GColorBabyBlueEyes,
        .smoke    = GColorBabyBlueEyes,
    },
    [THEME_FOREST] = {
        // the one it is named for: deep firs over a mossy clearing
        .readouts = {.text = GColorSpringBud, .dim = GColorInchworm},
        .tree_far = GColorArmyGreen,    .tree     = GColorMidnightGreen,
        .ground   = GColorDarkGreen,    .cabin    = GColorWindsorTan,
        .roof     = GColorArmyGreen,    .glow     = GColorIcterine,
        .smoke    = GColorWhite,
    },
    [THEME_MONO] = {
        .readouts = {.text = GColorWhite, .dim = GColorWhite},
        .tree_far = GColorLightGray,    .tree     = GColorDarkGray,
        .ground   = GColorDarkGray,     .cabin    = GColorLightGray,
        .roof     = GColorBlack,        .glow     = GColorWhite,
        .smoke    = GColorLightGray,
    },
    [THEME_CYBERPUNK] = {
        // a treeline that never switches its signage off, so the readouts stay on neon
        .readouts = {.text = GColorCyan, .dim = GColorMagenta},
        .tree_far = GColorImperialPurple, .tree     = GColorBlack,
        .ground   = GColorJazzberryJam, .cabin    = GColorImperialPurple,
        .roof     = GColorBlack,        .glow     = GColorCyan,
        .smoke    = GColorRajah,
    },
    [THEME_NEO_TOKYO] = {
        // slate firs under the haze
        .readouts = {.text = GColorWhite, .dim = GColorSunsetOrange},
        .tree_far = GColorCadetBlue,    .tree     = GColorOxfordBlue,
        .ground   = GColorLiberty,      .cabin    = GColorBulgarianRose,
        .roof     = GColorOxfordBlue,   .glow     = GColorSunsetOrange,
        .smoke    = GColorWhite,
    },
};

// after dark the ground goes dark enough for a light clock to sit straight on it. the window is
// lit in every one of them, which after dark is the warmest thing on the screen
static const TreelineGround s_night[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        .readouts = {.text = GColorWhite, .dim = GColorLightGray},
        .tree_far = GColorOxfordBlue,   .tree     = GColorBlack,
        .ground   = GColorBlack,        .cabin    = GColorOxfordBlue,
        .roof     = GColorBlack,        .glow     = GColorIcterine,
        .smoke    = GColorDarkGray,
    },
    [THEME_DAYBREAK] = {
        .readouts = {.text = GColorWhite, .dim = GColorBabyBlueEyes},
        .tree_far = GColorDukeBlue,     .tree     = GColorBlack,
        .ground   = GColorOxfordBlue,   .cabin    = GColorBulgarianRose,
        .roof     = GColorBlack,        .glow     = GColorRajah,
        .smoke    = GColorDarkGray,
    },
    [THEME_ALPENGLOW] = {
        .readouts = {.text = GColorMelon, .dim = GColorRichBrilliantLavender},
        .tree_far = GColorImperialPurple, .tree     = GColorBlack,
        .ground   = GColorBlack,        .cabin    = GColorImperialPurple,
        .roof     = GColorBlack,        .glow     = GColorIcterine,
        .smoke    = GColorImperialPurple,
    },
    [THEME_BLUEPRINT] = {
        .readouts = {.text = GColorWhite, .dim = GColorPictonBlue},
        .tree_far = GColorOxfordBlue,   .tree     = GColorBlack,
        .ground   = GColorBlack,        .cabin    = GColorOxfordBlue,
        .roof     = GColorBlack,        .glow     = GColorPictonBlue,
        .smoke    = GColorOxfordBlue,
    },
    [THEME_FOREST] = {
        .readouts = {.text = GColorSpringBud, .dim = GColorMayGreen},
        .tree_far = GColorMidnightGreen, .tree     = GColorBlack,
        .ground   = GColorArmyGreen,    .cabin    = GColorWindsorTan,
        .roof     = GColorBlack,        .glow     = GColorIcterine,
        .smoke    = GColorArmyGreen,
    },
    [THEME_MONO] = {
        .readouts = {.text = GColorWhite, .dim = GColorLightGray},
        .tree_far = GColorDarkGray,     .tree     = GColorBlack,
        .ground   = GColorBlack,        .cabin    = GColorDarkGray,
        .roof     = GColorBlack,        .glow     = GColorWhite,
        .smoke    = GColorDarkGray,
    },
    [THEME_CYBERPUNK] = {
        // black firs, with a cyan window burning in the cabin
        .readouts = {.text = GColorCyan, .dim = GColorMagenta},
        .tree_far = GColorJazzberryJam, .tree     = GColorBlack,
        .ground   = GColorBlack,        .cabin    = GColorImperialPurple,
        .roof     = GColorBlack,        .glow     = GColorCyan,
        .smoke    = GColorImperialPurple,
    },
    [THEME_NEO_TOKYO] = {
        // red neon bleeding up out of the trees, and it keeps white for the clock
        .readouts = {.text = GColorWhite, .dim = GColorSunsetOrange},
        .tree_far = GColorBulgarianRose, .tree     = GColorBlack,
        .ground   = GColorBlack,        .cabin    = GColorBulgarianRose,
        .roof     = GColorBlack,        .glow     = GColorSunsetOrange,
        .smoke    = GColorBulgarianRose,
    },
};

const void *treeline_ground_for_theme(uint8_t theme, bool night)
{
    if (theme >= THEME_COUNT)
    {
        theme = THEME_SKETCHBOOK;
    }

    return night ? &s_night[theme] : &s_day[theme];
}

/** @} */
