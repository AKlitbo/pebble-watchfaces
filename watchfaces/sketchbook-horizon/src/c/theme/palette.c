/**
 * @file palette.c
 * @brief The sky table every scene shares, a day and a night row for every theme, and the frame's
 * palette put together from it.
 *
 * @ingroup watchface-sketchbook
 */
#include "theme/palette.h"

#include <stddef.h>
#include <string.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// a CommonPalette is the sky row and then the ground's readouts, field for field, so the two copies
// in sketchbook_palette_assemble move every field across. a field added to one side and not the
// other, or added to both in a different place, moves something here and fails the build
#define SKY_LINES_UP(f) \
    _Static_assert(offsetof(CommonPalette, f) == offsetof(SkyPalette, f), #f " sits where the sky row has it")
#define READOUT_LINES_UP(f) \
    _Static_assert(offsetof(CommonPalette, f) == sizeof(SkyPalette) + offsetof(GroundReadouts, f), \
        #f " sits where the readouts have it")

SKY_LINES_UP(sky_hi);
SKY_LINES_UP(sky_lo);
SKY_LINES_UP(ink);
SKY_LINES_UP(bar_ink);
SKY_LINES_UP(disc);
SKY_LINES_UP(cloud);
SKY_LINES_UP(precip);
SKY_LINES_UP(star);
SKY_LINES_UP(batt_low);
SKY_LINES_UP(batt_critical);
READOUT_LINES_UP(text);
READOUT_LINES_UP(dim);
_Static_assert(sizeof(CommonPalette) == sizeof(SkyPalette) + sizeof(GroundReadouts),
    "a CommonPalette is exactly a sky row and the readouts");

// day skies. the sky is light, so the linework over it goes dark. the stars only come out at
// night, so `star` here just repeats the night colour
static const SkyPalette s_day[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        // pen on paper: a white sky under black linework
        .sky_hi = GColorWhite,          .sky_lo = GColorWhite,
        .ink    = GColorBlack,          .bar_ink = GColorWhite,
        .disc   = GColorYellow,         .cloud  = GColorWhite,
        .precip = GColorBlue,           .star   = GColorLightGray,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
    [THEME_DAYBREAK] = {
        .sky_hi = GColorVividCerulean,  .sky_lo = GColorCeleste,
        .ink    = GColorBlack,          .bar_ink = GColorMintGreen,
        .disc   = GColorYellow,         .cloud  = GColorWhite,
        .precip = GColorBlueMoon,       .star   = GColorBabyBlueEyes,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
    [THEME_ALPENGLOW] = {
        .sky_hi = GColorOrange,         .sky_lo = GColorRajah,
        .ink    = GColorBlack,          .bar_ink = GColorMelon,
        .disc   = GColorYellow,         .cloud  = GColorMelon,
        .precip = GColorElectricBlue,   .star   = GColorRichBrilliantLavender,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
    [THEME_BLUEPRINT] = {
        // white linework on blue paper
        .sky_hi = GColorDukeBlue,       .sky_lo = GColorCobaltBlue,
        .ink    = GColorWhite,          .bar_ink = GColorBabyBlueEyes,
        .disc   = GColorWhite,          .cloud  = GColorCobaltBlue,
        .precip = GColorBabyBlueEyes,   .star   = GColorPictonBlue,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
    [THEME_FOREST] = {
        .sky_hi = GColorCeleste,        .sky_lo = GColorMintGreen,
        .ink    = GColorBlack,          .bar_ink = GColorInchworm,
        .disc   = GColorIcterine,       .cloud  = GColorWhite,
        .precip = GColorTiffanyBlue,    .star   = GColorMayGreen,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
    [THEME_MONO] = {
        // the sun goes solid black here rather than pale: a white disc on white paper would
        // leave nothing but its outline, and the rays are drawn in the disc colour
        // the battery warnings match bar_ink by hand, so the gauge never changes colour and the
        // count of lit segments carries the charge
        .sky_hi = GColorWhite,          .sky_lo = GColorWhite,
        .ink    = GColorBlack,          .bar_ink = GColorLightGray,
        .disc   = GColorBlack,          .cloud  = GColorLightGray,
        .precip = GColorBlack,          .star   = GColorLightGray,
        .batt_low = GColorLightGray,    .batt_critical = GColorLightGray,
    },
    [THEME_CYBERPUNK] = {
        // daylight here is smog, not weather: a sick yellow sky over a scene that never
        // switches its signage off
        .sky_hi = GColorYellow,         .sky_lo = GColorChromeYellow,
        .ink    = GColorBlack,          .bar_ink = GColorMagenta,
        .disc   = GColorWhite,          .cloud  = GColorRajah,
        .precip = GColorCyan,           .star   = GColorMagenta,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
    [THEME_NEO_TOKYO] = {
        // a pale hazy sky with a flat red sun
        .sky_hi = GColorLightGray,      .sky_lo = GColorWhite,
        .ink    = GColorBlack,          .bar_ink = GColorSunsetOrange,
        .disc   = GColorRed,            .cloud  = GColorWhite,
        .precip = GColorCobaltBlue,     .star   = GColorSunsetOrange,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
};

// night skies. the sky drops to near-black, so the linework over it turns pale
static const SkyPalette s_night[THEME_COUNT] = {
    [THEME_SKETCHBOOK] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .ink    = GColorWhite,          .bar_ink = GColorLightGray,
        .disc   = GColorPastelYellow,   .cloud  = GColorDarkGray,
        .precip = GColorElectricBlue,   .star   = GColorLightGray,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
    [THEME_DAYBREAK] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .ink    = GColorCobaltBlue,     .bar_ink = GColorBabyBlueEyes,
        .disc   = GColorWhite,          .cloud  = GColorDarkGray,
        .precip = GColorElectricBlue,   .star   = GColorBabyBlueEyes,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
    [THEME_ALPENGLOW] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorIndigo,
        .ink    = GColorPurpureus,      .bar_ink = GColorRichBrilliantLavender,
        .disc   = GColorPastelYellow,   .cloud  = GColorImperialPurple,
        .precip = GColorLavenderIndigo, .star   = GColorRichBrilliantLavender,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
    [THEME_BLUEPRINT] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .ink    = GColorBabyBlueEyes,   .bar_ink = GColorPictonBlue,
        .disc   = GColorWhite,          .cloud  = GColorOxfordBlue,
        .precip = GColorPictonBlue,     .star   = GColorPictonBlue,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
    [THEME_FOREST] = {
        .sky_hi = GColorBlack,          .sky_lo = GColorMidnightGreen,
        .ink    = GColorJaegerGreen,    .bar_ink = GColorMayGreen,
        .disc   = GColorPastelYellow,   .cloud  = GColorArmyGreen,
        .precip = GColorTiffanyBlue,    .star   = GColorMayGreen,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
    [THEME_MONO] = {
        // the battery warnings match bar_ink by hand, so the gauge never changes colour and the
        // count of lit segments carries the charge
        .sky_hi = GColorBlack,          .sky_lo = GColorBlack,
        .ink    = GColorWhite,          .bar_ink = GColorLightGray,
        .disc   = GColorWhite,          .cloud  = GColorDarkGray,
        .precip = GColorWhite,          .star   = GColorLightGray,
        .batt_low = GColorLightGray,    .batt_critical = GColorLightGray,
    },
    [THEME_CYBERPUNK] = {
        // the one it is really for: magenta linework and a cyan moon over black
        .sky_hi = GColorBlack,          .sky_lo = GColorIndigo,
        .ink    = GColorMagenta,        .bar_ink = GColorMagenta,
        .disc   = GColorCyan,           .cloud  = GColorImperialPurple,
        .precip = GColorCyan,           .star   = GColorMagenta,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
    [THEME_NEO_TOKYO] = {
        // red linework into a deep blue night, with a cold white moon over it. warmer and
        // quieter than Cyberpunk
        .sky_hi = GColorBlack,          .sky_lo = GColorOxfordBlue,
        .ink    = GColorRed,            .bar_ink = GColorSunsetOrange,
        .disc   = GColorWhite,          .cloud  = GColorBulgarianRose,
        .precip = GColorSunsetOrange,   .star   = GColorSunsetOrange,
        .batt_low = GColorChromeYellow, .batt_critical = GColorRed,
    },
};

void sketchbook_palette_assemble(CommonPalette *out, uint8_t theme, bool night, const GroundReadouts *ground)
{
    if (theme >= THEME_COUNT)
    {
        theme = THEME_SKETCHBOOK;
    }

    const SkyPalette *sky = night ? &s_night[theme] : &s_day[theme];

    memcpy(out, sky, sizeof(*sky));
    memcpy(&out->text, ground, sizeof(*ground));
}

/** @} */
