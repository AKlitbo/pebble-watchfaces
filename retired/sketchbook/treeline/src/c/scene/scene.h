/**
 * @file scene.h
 * @brief The drawn forest: sky bands, the sun or moon on its arc, two treelines, a log cabin
 * with a lit window, and the smoke off its chimney leaning on the wind.
 *
 * Treeline has no baked background. Everything above the clock is painted here from the time,
 * the sunrise and sunset the phone sends, the wind it reports, and the current condition, which
 * is what lets the same clearing read as a still summer morning or a gale after dark.
 *
 * @ingroup watchface-treeline
 */
#pragma once
#include <pebble.h>

#include "theme/theme.h"

/**
 * @addtogroup watchface-treeline
 * @{
 */

// --- Scene geometry ---
// where the sky, the weather and the arc sit is in sketchbook_config.h, which is what the shared
// drawing code reads. only what this face draws for itself lives here
//
// the round screen is 30% wider and only 14% taller, so the forest is not a straight scale of the
// rectangle: the treelines spread to the new width and drop far enough that the clearing still
// has the clock's whole band under them
#if defined(PBL_ROUND)
#define FAR_BASE 126    ///< Where the back treeline stands
#define NEAR_BASE 145   ///< Where the front treeline and the cabin stand
#define GROUND_Y 138    ///< Top of the clearing floor. the near trunks stand down into it
#else
#define FAR_BASE 113
#define NEAR_BASE 129
#define GROUND_Y 123
#endif

/**
 * @brief Paint the whole scene: sky, stars, arc, disc, treelines, cabin, smoke and weather.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 */
void scene_draw(GContext *ctx, GRect bounds, const Palette *pal);

/** @} */
