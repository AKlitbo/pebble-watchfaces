/**
 * @file theme.c
 * @brief The palette table: a day and a night set of colours for every theme.
 *
 * The same eight themes Ridgeline has, since the two faces are a pair and picking between them
 * should not mean learning a second list of names. The sky, the linework and the accents are
 * that face's, and what differs is only what the ground is made of: sea, wet sand and dry sand
 * here where it has two ridges there.
 *
 * @ingroup watchface-shoreline
 */
#include "theme.h"

/**
 * @addtogroup watchface-shoreline
 * @{
 */

// day palettes. the sky is light, so the linework goes dark and the clock reads against the sand
// it sits on. the bands run sky, far water, near water, wet sand, dry sand from the top down, and
// neighbouring bands are kept apart or the waterline stops reading
static const Palette s_day[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        // pen on paper: the water is white too, and only the horizon, the wave marks and the
        // surf line say where the sea is
        .sky_hi = GColorWhite,          .sky_lo = GColorWhite,
        .sea_far = GColorLightGray,     .sea    = GColorWhite,
        .sand   = GColorWhite,          .wet    = GColorLightGray,
        .foam   = GColorBlack,          .ink    = GColorBlack,
        .text   = GColorBlack,          .dim    = GColorDarkGray,
        .bar_ink = GColorWhite,         .disc   = GColorYellow,
        .cloud  = GColorWhite,          .precip = GColorBlue,
    },
    [THEME_DAYBREAK] = {
        .sky_hi = GColorVividCerulean,  .sky_lo = GColorCeleste,
        .sea_far = GColorJaegerGreen,   .sea    = GColorMediumAquamarine,
        .sand   = GColorPastelYellow,   .wet    = GColorBrass,
        .foam   = GColorWhite,          .ink    = GColorBlack,
        .text   = GColorBlack,          .dim    = GColorDarkGreen,
        .bar_ink = GColorMintGreen,     .disc   = GColorYellow,
        .cloud  = GColorWhite,          .precip = GColorBlueMoon,
    },
    [THEME_ALPENGLOW] = {
        .sky_hi = GColorOrange,         .sky_lo = GColorRajah,
        .sea_far = GColorImperialPurple, .sea   = GColorBulgarianRose,
        .sand   = GColorMelon,          .wet    = GColorRoseVale,
        .foam   = GColorWhite,          .ink    = GColorBlack,
        .text   = GColorBulgarianRose,  .dim    = GColorImperialPurple,
        .bar_ink = GColorMelon,         .disc   = GColorYellow,
        .cloud  = GColorMelon,          .precip = GColorElectricBlue,
    },
    [THEME_BLUEPRINT] = {
        // white linework on blue paper, so the beach is drawn rather than coloured in
        .sky_hi = GColorDukeBlue,       .sky_lo = GColorCobaltBlue,
        .sea_far = GColorDukeBlue,      .sea    = GColorCobaltBlue,
        .sand   = GColorOxfordBlue,     .wet    = GColorDukeBlue,
        .foam   = GColorWhite,          .ink    = GColorWhite,
        .text   = GColorWhite,          .dim    = GColorBabyBlueEyes,
        .bar_ink = GColorBabyBlueEyes,  .disc   = GColorWhite,
        .cloud  = GColorCobaltBlue,     .precip = GColorBabyBlueEyes,
    },
    [THEME_FOREST] = {
        // the water is teal rather than green here: this theme inks in DarkGreen, and green
        // water swallowed the boat's hull whole
        .sky_hi = GColorCeleste,        .sky_lo = GColorMintGreen,
        .sea_far = GColorMidnightGreen, .sea    = GColorTiffanyBlue,
        .sand   = GColorMidnightGreen,  .wet    = GColorArmyGreen,
        .foam   = GColorWhite,          .ink    = GColorDarkGreen,
        .text   = GColorSpringBud,      .dim    = GColorInchworm,
        .bar_ink = GColorInchworm,      .disc   = GColorIcterine,
        .cloud  = GColorWhite,          .precip = GColorTiffanyBlue,
    },
    [THEME_MONO] = {
        // the sun goes solid black here rather than pale: a white disc on white paper would
        // leave nothing but its outline, and the rays are drawn in the disc colour
        .sky_hi = GColorWhite,          .sky_lo = GColorWhite,
        .sea_far = GColorWhite,         .sea    = GColorLightGray,
        .sand   = GColorDarkGray,       .wet    = GColorBlack,
        .foam   = GColorWhite,          .ink    = GColorBlack,
        .text   = GColorWhite,          .dim    = GColorWhite,
        .bar_ink = GColorLightGray,     .disc   = GColorBlack,
        .cloud  = GColorLightGray,      .precip = GColorBlack,
    },
    [THEME_CYBERPUNK] = {
        // daylight here is smog, not weather: a sick yellow sky over a seafront that never
        // switches its signage off, so the readouts stay on neon rather than going dark
        .sky_hi = GColorYellow,         .sky_lo = GColorChromeYellow,
        .sea_far = GColorImperialPurple, .sea   = GColorJazzberryJam,
        .sand   = GColorBlack,          .wet    = GColorImperialPurple,
        .foam   = GColorMagenta,        .ink    = GColorBlack,
        .text   = GColorCyan,           .dim    = GColorMagenta,
        .bar_ink = GColorMagenta,       .disc   = GColorWhite,
        .cloud  = GColorRajah,          .precip = GColorCyan,
    },
    [THEME_NEO_TOKYO] = {
        // a pale hazy sky with a flat red sun over slate water and a dark shore
        .sky_hi = GColorLightGray,      .sky_lo = GColorWhite,
        .sea_far = GColorCadetBlue,     .sea    = GColorLiberty,
        .sand   = GColorOxfordBlue,     .wet    = GColorBlack,
        .foam   = GColorWhite,          .ink    = GColorBlack,
        .text   = GColorWhite,          .dim    = GColorSunsetOrange,
        .bar_ink = GColorSunsetOrange,  .disc   = GColorRed,
        .cloud  = GColorWhite,          .precip = GColorCobaltBlue,
    },
};

// night palettes. the sky drops to near-black, so the linework turns pale and the sand goes dark
// enough for a light clock to sit straight on it. the bands alternate rather than all going black,
// which is the only thing keeping the water apart from the beach after dark
static const Palette s_night[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .sea_far = GColorOxfordBlue,    .sea    = GColorBlack,
        .sand   = GColorBlack,          .wet    = GColorOxfordBlue,
        .foam   = GColorWhite,          .ink    = GColorWhite,
        .text   = GColorWhite,          .dim    = GColorLightGray,
        .bar_ink = GColorLightGray,     .disc   = GColorPastelYellow,
        .cloud  = GColorDarkGray,       .precip = GColorElectricBlue,
    },
    [THEME_DAYBREAK] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .sea_far = GColorDukeBlue,      .sea    = GColorOxfordBlue,
        .sand   = GColorBlack,          .wet    = GColorDukeBlue,
        .foam   = GColorBabyBlueEyes,   .ink    = GColorCobaltBlue,
        .text   = GColorWhite,          .dim    = GColorBabyBlueEyes,
        .bar_ink = GColorBabyBlueEyes,  .disc   = GColorWhite,
        .cloud  = GColorDarkGray,       .precip = GColorElectricBlue,
    },
    [THEME_ALPENGLOW] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorIndigo,
        .sea_far = GColorImperialPurple, .sea   = GColorBlack,
        .sand   = GColorBlack,          .wet    = GColorImperialPurple,
        .foam   = GColorRichBrilliantLavender, .ink = GColorPurpureus,
        .text   = GColorMelon,          .dim    = GColorRichBrilliantLavender,
        .bar_ink = GColorRichBrilliantLavender, .disc = GColorPastelYellow,
        .cloud  = GColorImperialPurple, .precip = GColorLavenderIndigo,
    },
    [THEME_BLUEPRINT] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .sea_far = GColorOxfordBlue,    .sea    = GColorBlack,
        .sand   = GColorBlack,          .wet    = GColorOxfordBlue,
        .foam   = GColorPictonBlue,     .ink    = GColorBabyBlueEyes,
        .text   = GColorWhite,          .dim    = GColorPictonBlue,
        .bar_ink = GColorPictonBlue,    .disc   = GColorWhite,
        .cloud  = GColorOxfordBlue,     .precip = GColorPictonBlue,
    },
    [THEME_FOREST] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorMidnightGreen,
        .sea_far = GColorMidnightGreen, .sea    = GColorBlack,
        .sand   = GColorBlack,          .wet    = GColorArmyGreen,
        .foam   = GColorMintGreen,      .ink    = GColorJaegerGreen,
        .text   = GColorSpringBud,      .dim    = GColorMayGreen,
        .bar_ink = GColorMayGreen,      .disc   = GColorPastelYellow,
        .cloud  = GColorArmyGreen,      .precip = GColorTiffanyBlue,
    },
    [THEME_MONO] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorBlack,
        .sea_far = GColorDarkGray,      .sea    = GColorBlack,
        .sand   = GColorBlack,          .wet    = GColorDarkGray,
        .foam   = GColorWhite,          .ink    = GColorWhite,
        .text   = GColorWhite,          .dim    = GColorLightGray,
        .bar_ink = GColorLightGray,     .disc   = GColorWhite,
        .cloud  = GColorDarkGray,       .precip = GColorWhite,
    },
    [THEME_CYBERPUNK] = {
        // the one it is really for: magenta surf and a cyan moon over black water, with the
        // clock in cyan so the two neons split the screen between them
        .sky_hi = GColorBlack,          .sky_lo = GColorIndigo,
        .sea_far = GColorJazzberryJam,  .sea    = GColorBlack,
        .sand   = GColorBlack,          .wet    = GColorImperialPurple,
        .foam   = GColorMagenta,        .ink    = GColorMagenta,
        .text   = GColorCyan,           .dim    = GColorMagenta,
        .bar_ink = GColorMagenta,       .disc   = GColorCyan,
        .cloud  = GColorImperialPurple, .precip = GColorCyan,
    },
    [THEME_NEO_TOKYO] = {
        // red neon bleeding up off the front into a deep blue night, with a cold white moon
        // over it. warmer and quieter than Cyberpunk, and it keeps white for the clock
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .sea_far = GColorBulgarianRose, .sea    = GColorBlack,
        .sand   = GColorBlack,          .wet    = GColorBulgarianRose,
        .foam   = GColorSunsetOrange,   .ink    = GColorRed,
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
