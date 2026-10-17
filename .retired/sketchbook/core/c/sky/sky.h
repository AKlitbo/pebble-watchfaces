/**
 * @file sky.h
 * @brief The sky every Sketchbook face draws above its own ground.
 *
 * Two bands blended into each other, a field of stars after dark, the dashed arc the day runs
 * along, and the sun or moon somewhere on it. All of it comes from the clock and from the
 * sunrise and sunset the phone sends, so a face gets a sky that agrees with the hour without
 * doing any of this itself.
 *
 * Where it all sits comes from the face's sketchbook_config.h. The star field does not: those
 * are hand-placed per face to sit above its own horizon, so the face keeps the table and hands
 * it over.
 *
 * @ingroup family-sketchbook
 */
#pragma once
#include <pebble.h>

#include "theme/theme.h"  // the face's Palette

/**
 * @addtogroup family-sketchbook
 * @{
 */

/** @brief One star: where it is and how big it burns. */
typedef struct
{
    int16_t x;
    int16_t y;
    uint8_t size; ///< 0 a faint point, 1 a brighter dot, 2 a twinkling cross
} SketchbookStar;

/**
 * @brief Whether it is after dark.
 *
 * Worked out from the sunrise and sunset the phone sends, falling back to a plain 06:00 to
 * 18:00 day when there is no reading yet. The condition token's "_NIGHT" marker is deliberately
 * not used: the arc has to agree with the palette, so both read the same clock.
 *
 * @return True once the sun is down.
 */
bool sketchbook_sky_night(void);

/**
 * @brief How far the sun (or the moon, after dark) has got along its arc.
 *
 * @return The progress from 0 at the left horizon to 100 at the right.
 */
int sketchbook_sky_arc_progress(void);

/**
 * @brief Where on the arc a given progress lands.
 *
 * A half-ellipse over the arc's baseline: progress 0 puts the disc on the left, 50 at the apex,
 * and 100 on the right.
 *
 * @param progress How far along, 0 to 100.
 * @return The disc's centre.
 */
GPoint sketchbook_sky_arc_point(int progress);

/**
 * @brief Fill the sky: the upper band, the lower band, and a stippled blend between them.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 */
void sketchbook_sky_draw_bands(GContext *ctx, GRect bounds, const Palette *pal);

/**
 * @brief Scatter a star field. Night only.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param stars The face's own field.
 * @param count How many there are.
 */
void sketchbook_sky_draw_stars(GContext *ctx, const Palette *pal, const SketchbookStar *stars, int count);

/**
 * @brief Draw the arc the disc travels, as a broken guide line.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
void sketchbook_sky_draw_arc(GContext *ctx, const Palette *pal);

/**
 * @brief Draw the sun, or the moon at tonight's phase.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param at The disc's centre.
 * @param night True to draw the moon rather than the sun.
 */
void sketchbook_sky_draw_disc(GContext *ctx, const Palette *pal, GPoint at, bool night);

/** @} */
