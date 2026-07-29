/**
 * @file theme.c
 * @brief The palette table: a day and a night set of colours for every theme.
 *
 * @ingroup watchface-ridgeline
 */
#include "theme.h"

/**
 * @addtogroup watchface-ridgeline
 * @{
 */

// day palettes. the sky is light, so the linework goes dark and the clock goes light against
// the land it sits on
static const Palette s_day[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        .sky_hi = GColorWhite,        .sky_lo = GColorWhite,
        .land_far = GColorLightGray,  .land   = GColorWhite,
        .ink    = GColorBlack,        .text   = GColorBlack,
        .dim    = GColorDarkGray,     .disc   = GColorYellow,
        .cloud  = GColorWhite,        .precip = GColorBlue,
        .bar_ink = GColorWhite,
    },
    [THEME_DAYBREAK] = {
        .sky_hi = GColorVividCerulean, .sky_lo = GColorCeleste,
        .land_far = GColorJaegerGreen, .land   = GColorDarkGreen,
        .ink    = GColorBlack,         .text   = GColorWhite,
        .dim    = GColorMintGreen,     .disc   = GColorYellow,
        .cloud  = GColorWhite,         .precip = GColorBlueMoon,
        .bar_ink = GColorMintGreen,
    },
    [THEME_ALPENGLOW] = {
        .sky_hi = GColorOrange,        .sky_lo = GColorRajah,
        .land_far = GColorRoseVale,    .land   = GColorBulgarianRose,
        .ink    = GColorBlack,         .text   = GColorRajah,
        .dim    = GColorMelon,         .disc   = GColorYellow,
        .cloud  = GColorMelon,         .precip = GColorElectricBlue,
        .bar_ink = GColorMelon,
    },
    [THEME_BLUEPRINT] = {
        .sky_hi = GColorDukeBlue,      .sky_lo = GColorCobaltBlue,
        .land_far = GColorCobaltBlue,  .land   = GColorOxfordBlue,
        .ink    = GColorWhite,         .text   = GColorWhite,
        .dim    = GColorBabyBlueEyes,  .disc   = GColorWhite,
        .cloud  = GColorCobaltBlue,    .precip = GColorBabyBlueEyes,
        .bar_ink = GColorBabyBlueEyes,
    },
    [THEME_FOREST] = {
        .sky_hi = GColorCeleste,       .sky_lo = GColorMintGreen,
        .land_far = GColorArmyGreen,   .land   = GColorMidnightGreen,
        .ink    = GColorDarkGreen,     .text   = GColorSpringBud,
        .dim    = GColorInchworm,      .disc   = GColorIcterine,
        .cloud  = GColorWhite,         .precip = GColorTiffanyBlue,
        .bar_ink = GColorInchworm,
    },
    [THEME_MONO] = {
        // the sun goes solid black here rather than pale: a white disc on white paper would
        // leave nothing but its outline, and the rays are drawn in the disc colour
        .sky_hi = GColorWhite,         .sky_lo = GColorWhite,
        .land_far = GColorLightGray,   .land   = GColorDarkGray,
        .ink    = GColorBlack,         .text   = GColorWhite,
        .dim    = GColorWhite,         .disc   = GColorBlack,
        .cloud  = GColorLightGray,     .precip = GColorBlack,
        .bar_ink = GColorWhite,
    },
    [THEME_CYBERPUNK] = {
        // daylight here is smog, not weather: a sick yellow sky over a city that never
        // switches its signage off, so the readouts stay on neon rather than going dark
        .sky_hi = GColorYellow,        .sky_lo = GColorChromeYellow,
        .land_far = GColorImperialPurple, .land = GColorBlack,
        .ink    = GColorBlack,         .text   = GColorCyan,
        .dim    = GColorMagenta,       .disc   = GColorWhite,
        .cloud  = GColorRajah,         .precip = GColorCyan,
        .bar_ink = GColorMagenta,
    },
    [THEME_NEO_TOKYO] = {
        // a pale hazy sky with a flat red sun over slate ridges
        .sky_hi = GColorLightGray,     .sky_lo = GColorWhite,
        .land_far = GColorCadetBlue,   .land   = GColorOxfordBlue,
        .ink    = GColorBlack,         .text   = GColorWhite,
        .dim    = GColorSunsetOrange,  .disc   = GColorRed,
        .cloud  = GColorWhite,         .precip = GColorCobaltBlue,
        .bar_ink = GColorSunsetOrange,
    },
};

// night palettes. the sky drops to near-black, so the linework turns pale and the land goes
// dark enough for a light clock to sit straight on it
static const Palette s_night[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        .sky_hi = GColorBlack,         .sky_lo = GColorOxfordBlue,
        .land_far = GColorOxfordBlue,  .land   = GColorBlack,
        .ink    = GColorWhite,         .text   = GColorWhite,
        .dim    = GColorLightGray,     .disc   = GColorPastelYellow,
        .cloud  = GColorDarkGray,      .precip = GColorElectricBlue,
        .bar_ink = GColorLightGray,
    },
    [THEME_DAYBREAK] = {
        .sky_hi = GColorBlack,         .sky_lo = GColorOxfordBlue,
        .land_far = GColorDukeBlue,    .land   = GColorBlack,
        .ink    = GColorCobaltBlue,    .text   = GColorWhite,
        .dim    = GColorBabyBlueEyes,  .disc   = GColorWhite,
        .cloud  = GColorDarkGray,      .precip = GColorElectricBlue,
        .bar_ink = GColorBabyBlueEyes,
    },
    [THEME_ALPENGLOW] = {
        .sky_hi = GColorBlack,             .sky_lo = GColorIndigo,
        .land_far = GColorImperialPurple,  .land   = GColorBlack,
        .ink    = GColorPurpureus,         .text   = GColorMelon,
        .dim    = GColorRichBrilliantLavender, .disc = GColorPastelYellow,
        .cloud  = GColorImperialPurple,    .precip = GColorLavenderIndigo,
        .bar_ink = GColorRichBrilliantLavender,
    },
    [THEME_BLUEPRINT] = {
        .sky_hi = GColorBlack,         .sky_lo = GColorOxfordBlue,
        .land_far = GColorOxfordBlue,  .land   = GColorBlack,
        .ink    = GColorBabyBlueEyes,  .text   = GColorWhite,
        .dim    = GColorPictonBlue,    .disc   = GColorWhite,
        .cloud  = GColorOxfordBlue,    .precip = GColorPictonBlue,
        .bar_ink = GColorPictonBlue,
    },
    [THEME_FOREST] = {
        .sky_hi = GColorBlack,         .sky_lo = GColorMidnightGreen,
        .land_far = GColorArmyGreen,   .land   = GColorBlack,
        .ink    = GColorJaegerGreen,   .text   = GColorSpringBud,
        .dim    = GColorMayGreen,      .disc   = GColorPastelYellow,
        .cloud  = GColorArmyGreen,     .precip = GColorTiffanyBlue,
        .bar_ink = GColorMayGreen,
    },
    [THEME_MONO] = {
        .sky_hi = GColorBlack,         .sky_lo = GColorBlack,
        .land_far = GColorDarkGray,    .land   = GColorBlack,
        .ink    = GColorWhite,         .text   = GColorWhite,
        .dim    = GColorLightGray,     .disc   = GColorWhite,
        .cloud  = GColorDarkGray,      .precip = GColorWhite,
        .bar_ink = GColorLightGray,
    },
    [THEME_CYBERPUNK] = {
        // the one it is really for: magenta linework and a cyan moon over black, with the
        // clock in cyan so the two neons split the screen between them
        .sky_hi = GColorBlack,         .sky_lo = GColorIndigo,
        .land_far = GColorJazzberryJam, .land  = GColorBlack,
        .ink    = GColorMagenta,       .text   = GColorCyan,
        .dim    = GColorMagenta,       .disc   = GColorCyan,
        .cloud  = GColorImperialPurple, .precip = GColorCyan,
        .bar_ink = GColorMagenta,
    },
    [THEME_NEO_TOKYO] = {
        // red neon bleeding up off the city into a deep blue night, with a cold white moon
        // over it. warmer and quieter than Cyberpunk, and it keeps white for the clock
        .sky_hi = GColorBlack,         .sky_lo = GColorOxfordBlue,
        .land_far = GColorBulgarianRose, .land = GColorBlack,
        .ink    = GColorRed,           .text   = GColorWhite,
        .dim    = GColorSunsetOrange,  .disc   = GColorWhite,
        .cloud  = GColorBulgarianRose, .precip = GColorSunsetOrange,
        .bar_ink = GColorSunsetOrange,
    },
};

const Palette *palette_for(uint8_t theme, bool night)
{
    if (theme >= THEME_COUNT)
    {
        theme = THEME_SKETCHBOOK;
    }

    return night ? &s_night[theme] : &s_day[theme];
}

GColor battery_fill_for(uint8_t theme, GColor ink, int level)
{
    // Mono keeps the gauge greyscale. the lit-segment count already shows the charge, so a
    // red or amber warning would only break the look
    if (theme == THEME_MONO)
    {
        return ink;
    }

    if (level <= 20)
    {
        return GColorRed;
    }

    if (level <= 40)
    {
        return GColorChromeYellow;
    }

    return ink;
}

/** @} */
