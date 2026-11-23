/**
 * @file palette.h
 * @brief The palette contract: the theme list the face offers, the sky every scene shares, and the
 * colours the shared code paints with.
 *
 * The sky, the linework over it, the disc, the weather, the status bar, and the battery warnings
 * come from one table every scene shares, one row per theme by day and by night. What a scene
 * stands on is its own. Each scene keeps a ground table with its own fields, led by the two colours
 * that read on that ground, `text` and `dim`.
 *
 * The engine puts the two together into one CommonPalette for the frame, which is what the shared
 * sky, weather, and chrome code paints with. A scene's own draws read that and its ground row.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Themes the Clay picker offers, in its option order.
 *
 * Every scene is themed from this one list of names. The count caps KNOWN_THEME in the settings
 * schema.
 */
typedef enum
{
    THEME_SKETCHBOOK = 0,
    THEME_DAYBREAK   = 1,
    THEME_ALPENGLOW  = 2,
    THEME_BLUEPRINT  = 3,
    THEME_FOREST     = 4,
    THEME_MONO       = 5,
    THEME_CYBERPUNK  = 6,
    THEME_NEO_TOKYO  = 7,
    THEME_COUNT
} SketchbookTheme;

/**
 * @brief One theme's sky at one time of day, the same over every scene.
 *
 * The battery warnings live here too. Colour themes warn in amber when the charge is low and red
 * when it is critical. Mono sets both to its `bar_ink`, in its rows in palette.c, so its gauge
 * never changes colour and the count of lit segments carries the charge.
 */
typedef struct SkyPalette
{
    GColor sky_hi;        ///< Sky at the top of the screen
    GColor sky_lo;        ///< Sky down where the scene meets it
    GColor ink;           ///< Linework over the sky: outlines and the guide arc
    GColor bar_ink;       ///< The status bar's contents, which sit on the dark strip
    GColor disc;          ///< Sun or moon fill, the sun's rays, and the bright stars
    GColor cloud;         ///< Cloud fill, and the weather wash
    GColor precip;        ///< Rain, sleet and snow
    GColor star;          ///< The faint stars, which sit back behind the bright ones
    GColor batt_low;      ///< The battery gauge at 40% or less
    GColor batt_critical; ///< The battery gauge at 20% or less
} SkyPalette;

/**
 * @brief The two colours that read on a scene's ground.
 *
 * Every scene's ground row leads with these, so the engine can read them without knowing which
 * scene is up.
 */
typedef struct GroundReadouts
{
    GColor text; ///< The clock, over the scene's ground
    GColor dim;  ///< The standard layout's date, the stats row, and any mark on the scene's ground
} GroundReadouts;

/**
 * @brief The frame's palette, put together by the engine from the sky row and the scene's ground.
 *
 * It is a SkyPalette followed by a GroundReadouts, field for field and in the same order, which
 * palette.c checks when it builds. A field added to one goes in the same place here.
 *
 * Split by what a colour sits *on* rather than by what draws it. `ink` is linework over the sky and
 * `text` is the clock down on the ground, and the two are rarely the same once the ground is dark.
 *
 * `dim` is the colour for anything drawn on the scene's ground. That covers the date in the standard
 * layout and the stats row, and marks such as tufts, trails, sign posts, ripples, and wakes. The
 * date-up layouts move the date onto the status bar, where it takes `bar_ink`. Each scene picks `dim`
 * so it reads on its own ground in every theme, so a new mark down there takes `dim` rather than
 * borrowing a colour picked for something else.
 */
typedef struct CommonPalette
{
    GColor sky_hi;        ///< Sky at the top of the screen
    GColor sky_lo;        ///< Sky down where the scene meets it
    GColor ink;           ///< Linework over the sky: outlines and the guide arc
    GColor bar_ink;       ///< The status bar's contents, which sit on the dark strip
    GColor disc;          ///< Sun or moon fill, the sun's rays, and the bright stars
    GColor cloud;         ///< Cloud fill, and the weather wash
    GColor precip;        ///< Rain, sleet and snow
    GColor star;          ///< The faint stars, which sit back behind the bright ones
    GColor batt_low;      ///< The battery gauge at 40% or less
    GColor batt_critical; ///< The battery gauge at 20% or less
    GColor text;          ///< The clock, over the scene's ground
    GColor dim;           ///< The standard layout's date, the stats row, and any mark on the ground
} CommonPalette;

/**
 * @brief Put a frame's palette together from a theme's sky and a scene's ground.
 *
 * An out-of-range theme falls back to Sketchbook, matching how the settings parser clamps.
 *
 * @param out Where the palette goes.
 * @param theme The palette on screen, which Shuffle or the night or Quiet Time palette can pick.
 * @param night True for the after-dark sky.
 * @param ground The scene's ground row for the same theme and time of day.
 */
void sketchbook_palette_assemble(CommonPalette *out, uint8_t theme, bool night, const GroundReadouts *ground);

/** @} */
