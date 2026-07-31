/**
 * @file theme.c
 * @brief The palette table: a day and a night set of colours for every theme.
 *
 * The same eight themes Ridgeline and Shoreline have, since the three are a set and picking
 * between them should not mean learning three lists of names. The sky, the linework and the
 * accents are shared, and what differs is only what the ground is made of: two treelines, a
 * cabin and a clearing here where one has ridges and the other has water.
 *
 * @ingroup watchface-treeline
 */
#include "theme.h"

/**
 * @addtogroup watchface-treeline
 * @{
 */

// day palettes. the sky is light, so the linework goes dark and the clock reads against the
// clearing it sits on. the window has no glow by day, so its daylight value is just the wall
// shade one step darker
static const Palette s_day[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        // pen on paper, so the ink goes in the near trees and the far ones are left as
        // outlines. the other way round is truer to a fading distance and gives the front of
        // the wood nothing to read as, which on a stand of firs is most of the picture
        .sky_hi = GColorWhite,          .sky_lo = GColorWhite,
        .tree_far = GColorWhite,        .tree   = GColorLightGray,
        .ground = GColorWhite,          .cabin  = GColorWhite,
        .roof   = GColorLightGray,      .glow   = GColorDarkGray,
        .smoke  = GColorLightGray,      .ink    = GColorBlack,
        .text   = GColorBlack,          .dim    = GColorDarkGray,
        .bar_ink = GColorWhite,         .disc   = GColorYellow,
        .cloud  = GColorWhite,          .precip = GColorBlue,
    },
    [THEME_DAYBREAK] = {
        .sky_hi = GColorVividCerulean,  .sky_lo = GColorCeleste,
        .tree_far = GColorJaegerGreen,  .tree   = GColorDarkGreen,
        .ground = GColorMayGreen,       .cabin  = GColorWindsorTan,
        .roof   = GColorBulgarianRose,  .glow   = GColorRajah,
        .smoke  = GColorWhite,          .ink    = GColorBlack,
        .text   = GColorBlack,          .dim    = GColorDarkGreen,
        .bar_ink = GColorMintGreen,     .disc   = GColorYellow,
        .cloud  = GColorWhite,          .precip = GColorBlueMoon,
    },
    [THEME_ALPENGLOW] = {
        .sky_hi = GColorOrange,         .sky_lo = GColorRajah,
        .tree_far = GColorBulgarianRose, .tree  = GColorImperialPurple,
        .ground = GColorMelon,          .cabin  = GColorRoseVale,
        .roof   = GColorBulgarianRose,  .glow   = GColorIcterine,
        .smoke  = GColorMelon,          .ink    = GColorBlack,
        .text   = GColorBulgarianRose,  .dim    = GColorImperialPurple,
        .bar_ink = GColorMelon,         .disc   = GColorYellow,
        .cloud  = GColorMelon,          .precip = GColorElectricBlue,
    },
    [THEME_BLUEPRINT] = {
        // white linework on blue paper, so the forest is drawn rather than coloured in
        .sky_hi = GColorDukeBlue,       .sky_lo = GColorCobaltBlue,
        .tree_far = GColorDukeBlue,     .tree   = GColorOxfordBlue,
        .ground = GColorCobaltBlue,     .cabin  = GColorDukeBlue,
        .roof   = GColorOxfordBlue,     .glow   = GColorBabyBlueEyes,
        .smoke  = GColorBabyBlueEyes,   .ink    = GColorWhite,
        .text   = GColorWhite,          .dim    = GColorBabyBlueEyes,
        .bar_ink = GColorBabyBlueEyes,  .disc   = GColorWhite,
        .cloud  = GColorCobaltBlue,     .precip = GColorBabyBlueEyes,
    },
    [THEME_FOREST] = {
        // the one it is named for: deep firs over a mossy clearing
        .sky_hi = GColorCeleste,        .sky_lo = GColorMintGreen,
        .tree_far = GColorArmyGreen,    .tree   = GColorMidnightGreen,
        .ground = GColorDarkGreen,      .cabin  = GColorWindsorTan,
        .roof   = GColorArmyGreen,      .glow   = GColorIcterine,
        .smoke  = GColorWhite,          .ink    = GColorBlack,
        .text   = GColorSpringBud,      .dim    = GColorInchworm,
        .bar_ink = GColorInchworm,      .disc   = GColorIcterine,
        .cloud  = GColorWhite,          .precip = GColorTiffanyBlue,
    },
    [THEME_MONO] = {
        // the sun goes solid black here rather than pale: a white disc on white paper would
        // leave nothing but its outline, and the rays are drawn in the disc colour
        .sky_hi = GColorWhite,          .sky_lo = GColorWhite,
        .tree_far = GColorLightGray,    .tree   = GColorDarkGray,
        .ground = GColorDarkGray,       .cabin  = GColorLightGray,
        .roof   = GColorBlack,          .glow   = GColorWhite,
        .smoke  = GColorLightGray,      .ink    = GColorBlack,
        .text   = GColorWhite,          .dim    = GColorWhite,
        .bar_ink = GColorLightGray,     .disc   = GColorBlack,
        .cloud  = GColorLightGray,      .precip = GColorBlack,
    },
    [THEME_CYBERPUNK] = {
        // daylight here is smog, not weather: a sick yellow sky over a treeline that never
        // switches its signage off, so the readouts stay on neon rather than going dark
        .sky_hi = GColorYellow,         .sky_lo = GColorChromeYellow,
        .tree_far = GColorImperialPurple, .tree = GColorBlack,
        .ground = GColorJazzberryJam,   .cabin  = GColorImperialPurple,
        .roof   = GColorBlack,          .glow   = GColorCyan,
        .smoke  = GColorRajah,          .ink    = GColorBlack,
        .text   = GColorCyan,           .dim    = GColorMagenta,
        .bar_ink = GColorMagenta,       .disc   = GColorWhite,
        .cloud  = GColorRajah,          .precip = GColorCyan,
    },
    [THEME_NEO_TOKYO] = {
        // a pale hazy sky with a flat red sun over slate firs
        .sky_hi = GColorLightGray,      .sky_lo = GColorWhite,
        .tree_far = GColorCadetBlue,    .tree   = GColorOxfordBlue,
        .ground = GColorLiberty,        .cabin  = GColorBulgarianRose,
        .roof   = GColorOxfordBlue,     .glow   = GColorSunsetOrange,
        .smoke  = GColorWhite,          .ink    = GColorBlack,
        .text   = GColorWhite,          .dim    = GColorSunsetOrange,
        .bar_ink = GColorSunsetOrange,  .disc   = GColorRed,
        .cloud  = GColorWhite,          .precip = GColorCobaltBlue,
    },
};

// night palettes. the sky drops to near-black, so the linework turns pale and the ground goes
// dark enough for a light clock to sit straight on it. the window is lit in every one of them,
// which after dark is the warmest thing on the screen
static const Palette s_night[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .tree_far = GColorOxfordBlue,   .tree   = GColorBlack,
        .ground = GColorBlack,          .cabin  = GColorOxfordBlue,
        .roof   = GColorBlack,          .glow   = GColorIcterine,
        .smoke  = GColorDarkGray,       .ink    = GColorWhite,
        .text   = GColorWhite,          .dim    = GColorLightGray,
        .bar_ink = GColorLightGray,     .disc   = GColorPastelYellow,
        .cloud  = GColorDarkGray,       .precip = GColorElectricBlue,
    },
    [THEME_DAYBREAK] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .tree_far = GColorDukeBlue,     .tree   = GColorBlack,
        .ground = GColorOxfordBlue,     .cabin  = GColorBulgarianRose,
        .roof   = GColorBlack,          .glow   = GColorRajah,
        .smoke  = GColorDarkGray,       .ink    = GColorCobaltBlue,
        .text   = GColorWhite,          .dim    = GColorBabyBlueEyes,
        .bar_ink = GColorBabyBlueEyes,  .disc   = GColorWhite,
        .cloud  = GColorDarkGray,       .precip = GColorElectricBlue,
    },
    [THEME_ALPENGLOW] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorIndigo,
        .tree_far = GColorImperialPurple, .tree = GColorBlack,
        .ground = GColorBlack,          .cabin  = GColorImperialPurple,
        .roof   = GColorBlack,          .glow   = GColorIcterine,
        .smoke  = GColorImperialPurple, .ink    = GColorPurpureus,
        .text   = GColorMelon,          .dim    = GColorRichBrilliantLavender,
        .bar_ink = GColorRichBrilliantLavender, .disc = GColorPastelYellow,
        .cloud  = GColorImperialPurple, .precip = GColorLavenderIndigo,
    },
    [THEME_BLUEPRINT] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .tree_far = GColorOxfordBlue,   .tree   = GColorBlack,
        .ground = GColorBlack,          .cabin  = GColorOxfordBlue,
        .roof   = GColorBlack,          .glow   = GColorPictonBlue,
        .smoke  = GColorOxfordBlue,     .ink    = GColorBabyBlueEyes,
        .text   = GColorWhite,          .dim    = GColorPictonBlue,
        .bar_ink = GColorPictonBlue,    .disc   = GColorWhite,
        .cloud  = GColorOxfordBlue,     .precip = GColorPictonBlue,
    },
    [THEME_FOREST] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorMidnightGreen,
        .tree_far = GColorMidnightGreen, .tree  = GColorBlack,
        .ground = GColorArmyGreen,      .cabin  = GColorWindsorTan,
        .roof   = GColorBlack,          .glow   = GColorIcterine,
        .smoke  = GColorArmyGreen,      .ink    = GColorJaegerGreen,
        .text   = GColorSpringBud,      .dim    = GColorMayGreen,
        .bar_ink = GColorMayGreen,      .disc   = GColorPastelYellow,
        .cloud  = GColorArmyGreen,      .precip = GColorTiffanyBlue,
    },
    [THEME_MONO] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorBlack,
        .tree_far = GColorDarkGray,     .tree   = GColorBlack,
        .ground = GColorBlack,          .cabin  = GColorDarkGray,
        .roof   = GColorBlack,          .glow   = GColorWhite,
        .smoke  = GColorDarkGray,       .ink    = GColorWhite,
        .text   = GColorWhite,          .dim    = GColorLightGray,
        .bar_ink = GColorLightGray,     .disc   = GColorWhite,
        .cloud  = GColorDarkGray,       .precip = GColorWhite,
    },
    [THEME_CYBERPUNK] = {
        // the one it is really for: magenta linework and a cyan moon over black firs, with a
        // cyan window burning in the cabin
        .sky_hi = GColorBlack,          .sky_lo = GColorIndigo,
        .tree_far = GColorJazzberryJam, .tree   = GColorBlack,
        .ground = GColorBlack,          .cabin  = GColorImperialPurple,
        .roof   = GColorBlack,          .glow   = GColorCyan,
        .smoke  = GColorImperialPurple, .ink    = GColorMagenta,
        .text   = GColorCyan,           .dim    = GColorMagenta,
        .bar_ink = GColorMagenta,       .disc   = GColorCyan,
        .cloud  = GColorImperialPurple, .precip = GColorCyan,
    },
    [THEME_NEO_TOKYO] = {
        // red neon bleeding up out of the trees into a deep blue night, with a cold white moon
        // over it. warmer and quieter than Cyberpunk, and it keeps white for the clock
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .tree_far = GColorBulgarianRose, .tree  = GColorBlack,
        .ground = GColorBlack,          .cabin  = GColorBulgarianRose,
        .roof   = GColorBlack,          .glow   = GColorSunsetOrange,
        .smoke  = GColorBulgarianRose,  .ink    = GColorRed,
        .text   = GColorWhite,          .dim    = GColorSunsetOrange,
        .bar_ink = GColorSunsetOrange,  .disc   = GColorWhite,
        .cloud  = GColorBulgarianRose,  .precip = GColorSunsetOrange,
    },
};

const Palette *palette_for_theme(uint8_t theme, bool night)
{
    if (theme >= THEME_COUNT)
    {
        theme = THEME_SKETCHBOOK;
    }

    return night ? &s_night[theme] : &s_day[theme];
}

/** @} */
