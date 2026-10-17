/**
 * @file skyline.c
 * @brief The Skyline palette table: a day and a night set of colours for every theme.
 *
 * @ingroup watchface-sketchbook
 */
#include "theme/skyline.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// day palettes. the sky is light, so the linework goes dark and the clock reads against the
// water it sits on. by day the lamps are mostly out, so `lamp` is carrying only the handful of
// windows that stay lit whatever the hour
static const SkylinePalette s_day[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        // pen on paper: the river is white too, and only the reflections say it is there
        .sky_hi = GColorWhite,          .sky_lo = GColorWhite,
        .city   = GColorLightGray,      .bridge = GColorDarkGray,
        .water  = GColorWhite,          .lamp   = GColorBlack,
        .ink    = GColorBlack,          .text   = GColorBlack,
        .dim    = GColorDarkGray,       .bar_ink = GColorWhite,
        .disc   = GColorYellow,         .cloud  = GColorWhite,
        .precip = GColorBlue,
    },
    [THEME_DAYBREAK] = {
        .sky_hi = GColorVividCerulean,  .sky_lo = GColorCeleste,
        .city   = GColorCadetBlue,      .bridge = GColorBulgarianRose,
        .water  = GColorBlueMoon,       .lamp   = GColorIcterine,
        .ink    = GColorBlack,          .text   = GColorWhite,
        .dim    = GColorBabyBlueEyes,   .bar_ink = GColorMintGreen,
        .disc   = GColorYellow,         .cloud  = GColorWhite,
        .precip = GColorBlueMoon,
    },
    [THEME_ALPENGLOW] = {
        .sky_hi = GColorOrange,         .sky_lo = GColorRajah,
        .city   = GColorRoseVale,       .bridge = GColorBulgarianRose,
        .water  = GColorImperialPurple, .lamp   = GColorIcterine,
        .ink    = GColorBlack,          .text   = GColorMelon,
        .dim    = GColorRichBrilliantLavender, .bar_ink = GColorMelon,
        .disc   = GColorYellow,         .cloud  = GColorMelon,
        .precip = GColorElectricBlue,
    },
    [THEME_BLUEPRINT] = {
        // white linework on blue paper, so the crossing is drawn rather than coloured in
        .sky_hi = GColorDukeBlue,       .sky_lo = GColorCobaltBlue,
        .city   = GColorDukeBlue,       .bridge = GColorOxfordBlue,
        .water  = GColorCobaltBlue,     .lamp   = GColorBabyBlueEyes,
        .ink    = GColorWhite,          .text   = GColorWhite,
        .dim    = GColorBabyBlueEyes,   .bar_ink = GColorBabyBlueEyes,
        .disc   = GColorWhite,          .cloud  = GColorCobaltBlue,
        .precip = GColorBabyBlueEyes,
    },
    [THEME_FOREST] = {
        .sky_hi = GColorCeleste,        .sky_lo = GColorMintGreen,
        .city   = GColorArmyGreen,      .bridge = GColorMidnightGreen,
        .water  = GColorDarkGreen,      .lamp   = GColorIcterine,
        .ink    = GColorBlack,          .text   = GColorSpringBud,
        .dim    = GColorInchworm,       .bar_ink = GColorInchworm,
        .disc   = GColorIcterine,       .cloud  = GColorWhite,
        .precip = GColorTiffanyBlue,
    },
    [THEME_MONO] = {
        // the sun goes solid black here rather than pale: a white disc on white paper would
        // leave nothing but its outline, and the rays are drawn in the disc colour
        .sky_hi = GColorWhite,          .sky_lo = GColorWhite,
        .city   = GColorLightGray,      .bridge = GColorBlack,
        .water  = GColorDarkGray,       .lamp   = GColorWhite,
        .ink    = GColorBlack,          .text   = GColorWhite,
        .dim    = GColorWhite,          .bar_ink = GColorLightGray,
        .disc   = GColorBlack,          .cloud  = GColorLightGray,
        .precip = GColorBlack,
    },
    [THEME_CYBERPUNK] = {
        // daylight here is smog, not weather: a sick yellow sky over a river that never
        // switches its signage off, so the readouts stay on neon rather than going dark
        .sky_hi = GColorYellow,         .sky_lo = GColorChromeYellow,
        .city   = GColorImperialPurple, .bridge = GColorBlack,
        .water  = GColorJazzberryJam,   .lamp   = GColorCyan,
        .ink    = GColorBlack,          .text   = GColorCyan,
        .dim    = GColorMagenta,        .bar_ink = GColorMagenta,
        .disc   = GColorWhite,          .cloud  = GColorRajah,
        .precip = GColorCyan,
    },
    [THEME_NEO_TOKYO] = {
        // a pale hazy sky with a flat red sun over a slate river
        .sky_hi = GColorLightGray,      .sky_lo = GColorWhite,
        .city   = GColorCadetBlue,      .bridge = GColorOxfordBlue,
        .water  = GColorLiberty,        .lamp   = GColorSunsetOrange,
        .ink    = GColorBlack,          .text   = GColorWhite,
        .dim    = GColorSunsetOrange,   .bar_ink = GColorSunsetOrange,
        .disc   = GColorRed,            .cloud  = GColorWhite,
        .precip = GColorCobaltBlue,
    },
};

// night palettes. the sky drops to near-black and the water with it, which is the whole point:
// a dark river is what makes a string of lamps and their reflections the brightest thing on the
// screen. the bridge itself goes almost to silhouette so the lights carry the picture
static const SkylinePalette s_night[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .city   = GColorOxfordBlue,     .bridge = GColorBlack,
        .water  = GColorBlack,          .lamp   = GColorIcterine,
        .ink    = GColorWhite,          .text   = GColorWhite,
        .dim    = GColorLightGray,      .bar_ink = GColorLightGray,
        .disc   = GColorPastelYellow,   .cloud  = GColorDarkGray,
        .precip = GColorElectricBlue,
    },
    [THEME_DAYBREAK] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .city   = GColorDukeBlue,       .bridge = GColorBlack,
        .water  = GColorOxfordBlue,     .lamp   = GColorRajah,
        .ink    = GColorCobaltBlue,     .text   = GColorWhite,
        .dim    = GColorBabyBlueEyes,   .bar_ink = GColorBabyBlueEyes,
        .disc   = GColorWhite,          .cloud  = GColorDarkGray,
        .precip = GColorElectricBlue,
    },
    [THEME_ALPENGLOW] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorIndigo,
        .city   = GColorImperialPurple, .bridge = GColorBlack,
        .water  = GColorBlack,          .lamp   = GColorIcterine,
        .ink    = GColorPurpureus,      .text   = GColorMelon,
        .dim    = GColorRichBrilliantLavender, .bar_ink = GColorRichBrilliantLavender,
        .disc   = GColorPastelYellow,   .cloud  = GColorImperialPurple,
        .precip = GColorLavenderIndigo,
    },
    [THEME_BLUEPRINT] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .city   = GColorOxfordBlue,     .bridge = GColorBlack,
        .water  = GColorOxfordBlue,     .lamp   = GColorPictonBlue,
        .ink    = GColorBabyBlueEyes,   .text   = GColorWhite,
        .dim    = GColorPictonBlue,     .bar_ink = GColorPictonBlue,
        .disc   = GColorWhite,          .cloud  = GColorOxfordBlue,
        .precip = GColorPictonBlue,
    },
    [THEME_FOREST] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorMidnightGreen,
        .city   = GColorMidnightGreen,  .bridge = GColorBlack,
        .water  = GColorBlack,          .lamp   = GColorIcterine,
        .ink    = GColorJaegerGreen,    .text   = GColorSpringBud,
        .dim    = GColorMayGreen,       .bar_ink = GColorMayGreen,
        .disc   = GColorPastelYellow,   .cloud  = GColorArmyGreen,
        .precip = GColorTiffanyBlue,
    },
    [THEME_MONO] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorBlack,
        .city   = GColorDarkGray,       .bridge = GColorBlack,
        .water  = GColorBlack,          .lamp   = GColorWhite,
        .ink    = GColorWhite,          .text   = GColorWhite,
        .dim    = GColorLightGray,      .bar_ink = GColorLightGray,
        .disc   = GColorWhite,          .cloud  = GColorDarkGray,
        .precip = GColorWhite,
    },
    [THEME_CYBERPUNK] = {
        // the one it is really for: a black river under magenta linework, with every lamp and
        // every reflection burning cyan
        .sky_hi = GColorBlack,          .sky_lo = GColorIndigo,
        .city   = GColorJazzberryJam,   .bridge = GColorBlack,
        .water  = GColorBlack,          .lamp   = GColorCyan,
        .ink    = GColorMagenta,        .text   = GColorCyan,
        .dim    = GColorMagenta,        .bar_ink = GColorMagenta,
        .disc   = GColorCyan,           .cloud  = GColorImperialPurple,
        .precip = GColorCyan,
    },
    [THEME_NEO_TOKYO] = {
        // red neon bleeding off the far bank into a deep blue night, with a cold white moon
        // over it. warmer and quieter than Cyberpunk, and it keeps white for the clock
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .city   = GColorBulgarianRose,  .bridge = GColorBlack,
        .water  = GColorBlack,          .lamp   = GColorSunsetOrange,
        .ink    = GColorRed,            .text   = GColorWhite,
        .dim    = GColorSunsetOrange,   .bar_ink = GColorSunsetOrange,
        .disc   = GColorWhite,          .cloud  = GColorBulgarianRose,
        .precip = GColorSunsetOrange,
    },
};

const SkylinePalette *skyline_palette_for_theme(uint8_t theme, bool night)
{
    if (theme >= THEME_COUNT)
    {
        theme = THEME_SKETCHBOOK;
    }

    return night ? &s_night[theme] : &s_day[theme];
}

/** @} */
