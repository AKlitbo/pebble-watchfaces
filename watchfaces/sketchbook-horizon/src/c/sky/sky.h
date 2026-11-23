/**
 * @file sky.h
 * @brief The sky the layout engine paints over every scene's ground.
 *
 * Two bands blended into each other, a field of stars after dark, the dashed arc the day runs
 * along, and the sun or moon somewhere on it. All of it comes from the clock and from the
 * sunrise and sunset the phone sends, so every scene gets a sky that agrees with the hour without
 * doing any of this itself.
 *
 * Where it all sits comes from the scene's SceneGeometry in scene/geometry.h, star field and all.
 * Each scene places its stars by hand to sit above its own horizon, and the geometry carries them.
 *
 * The lights a scene switches on at dusk follow the same sunrise and sunset, so how lit the evening
 * is lives here too.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "scene/frame.h"

/**
 * @addtogroup watchface-sketchbook
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
 * @brief Paint the sky: the two bands and their blend, the stars after dark, the arc, and the sun
 * or moon unless the cloud hides it.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading.
 */
void sketchbook_sky_draw(GContext *ctx, GRect bounds, const SceneFrame *frame);

/**
 * @brief How lit a place is: 0 by day, up to 100 early in the evening, then thinning to about 55
 * by dawn.
 *
 * The lights come on quickly at dusk, hold through the evening, and thin out as the night wears
 * on, which is what makes a scene read as a place with people in it rather than a light sensor.
 *
 * @return 0 to 100.
 */
int sketchbook_sky_lit(void);

/**
 * @brief A steady place in the queue of lights coming on, 0 to 99.
 *
 * Every window and lamp keeps its own place, so a city fills up the same way each evening rather
 * than flickering from minute to minute.
 *
 * @param a First coordinate of the light, such as its block.
 * @param b Second, such as which window of that block.
 * @return The light's place in the queue.
 */
int sketchbook_sky_light_rank(int a, int b);

/** @} */
