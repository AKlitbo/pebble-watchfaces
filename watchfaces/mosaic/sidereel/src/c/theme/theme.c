/**
 * @file theme.c
 * @brief The four themes: the panel palette they start from, and the face colours they paint.
 *
 * Whichever row is running, the reel and pointer colours can be overridden from the config page,
 * so the table is where a theme starts rather than always where it ends.
 *
 * @ingroup watchface-sidereel
 */
#include "theme.h"

#include "settings_schema.h"
#include "system/settings/settings.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

// one row per theme, in enum order. the reel is the hero on every one of them, so what really
// changes down this table is the accent: mono keeps the pointer white, vibrant gives it the
// face's own red, and the inverse flips the whole thing onto white
static const Chrome s_chrome[THEME_COUNT] = {
    [THEME_MONO] = {
        .background = GColorBlack,     .panel = GColorWhite,
        .reel_ink = GColorBlack,       .highlight = GColorLightGray,
        .pointer = GColorWhite,        .pointer_ink = GColorBlack,
        .band_day = GColorWhite,       .band_night = GColorDarkGray,
        .band_now = GColorLightGray,
    },
    [THEME_VIBRANT] = {
        .background = GColorBlack,     .panel = GColorWhite,
        .reel_ink = GColorBlack,       .highlight = GColorLightGray,
        .pointer = GColorRed,          .pointer_ink = GColorWhite,
        .band_day = GColorYellow,      .band_night = GColorImperialPurple,
        .band_now = GColorRed,
    },
    // custom colours the panels, so the face around them stays out of the way
    [THEME_CUSTOM] = {
        .background = GColorBlack,     .panel = GColorWhite,
        .reel_ink = GColorBlack,       .highlight = GColorLightGray,
        .pointer = GColorWhite,        .pointer_ink = GColorBlack,
        .band_day = GColorWhite,       .band_night = GColorDarkGray,
        .band_now = GColorLightGray,
    },
    [THEME_MONO_INVERSE] = {
        .background = GColorWhite,     .panel = GColorBlack,
        .reel_ink = GColorWhite,       .highlight = GColorDarkGray,
        .pointer = GColorBlack,        .pointer_ink = GColorWhite,
        .band_day = GColorBlack,       .band_night = GColorLightGray,
        .band_now = GColorDarkGray,
    },
};

// latched by theme_refresh rather than read per draw, so a repaint costs one pointer hop
static const Chrome *s_live = &s_chrome[THEME_MONO];

// the running theme's row with the face colours painted over it, built at refresh rather than
// sitting in the table above because four of its colours come from the config page
static Chrome s_overridden;

Palette palette_for_theme(uint8_t theme)
{
    if (theme == THEME_MONO_INVERSE)
    {
        // the flip side. black on white with a darker grey for the dim caption line
        return (Palette){
            .accent = GColorBlack,
            .value = GColorBlack,
            .subtitle = GColorDarkGray,
            .icon = GColorBlack,
        };
    }

    // every other theme starts out mono. the colour comes from the panel overrides that
    // panel.c only paints on under VIBRANT and CUSTOM
    return (Palette){
        .accent = GColorWhite,
        .value = GColorWhite,
        .subtitle = GColorLightGray,
        .icon = GColorWhite,
    };
}

GColor theme_background(uint8_t theme)
{
    return theme == THEME_MONO_INVERSE ? GColorWhite : GColorBlack;
}

void theme_refresh(void)
{
    uint8_t theme = settings_u8(SETTING_THEME);

    if (theme >= THEME_COUNT)
    {
        theme = THEME_MONO;
    }

    if (!sidereel_face_colors())
    {
        s_live = &s_chrome[theme];
        return;
    }

    // the picks go over whichever theme is running rather than belonging to one of them, so a
    // mono face can still carry a red pointer
    s_overridden = s_chrome[theme];
    s_overridden.pointer = sidereel_pointer_color();
    s_overridden.pointer_ink = sidereel_pointer_ink();
    s_overridden.panel = sidereel_reel_color();
    s_overridden.reel_ink = sidereel_reel_ink();
    s_live = &s_overridden;
}

const Chrome *theme_chrome(void)
{
    return s_live;
}

/** @} */
