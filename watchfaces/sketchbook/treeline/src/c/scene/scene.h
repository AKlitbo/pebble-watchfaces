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
#define SKY_BAND_Y 46   ///< Where the upper sky starts giving way to the lower
#define SKY_BLEND_H 24  ///< How far it takes to get there, stippled so there is no hard seam
#define HORIZON_Y 108   ///< The arc's baseline, behind the trees, so the sun sets into the forest
#define ARC_CX 100      ///< Centre of the sun's arc across the sky
#define ARC_RX 86       ///< How far the arc reaches either side of centre
#define ARC_RY 72       ///< How high the arc climbs, kept under the status bar
#define DISC_R 12       ///< Radius of the sun and moon

#define FAR_BASE 113    ///< Where the back treeline stands
#define NEAR_BASE 129   ///< Where the front treeline and the cabin stand
#define GROUND_Y 123    ///< Top of the clearing floor. the near trunks stand down into it

#define MIST_TOP 81     ///< Where the mist wash starts thinning out
#define MIST_BOTTOM 129 ///< Where it stops, just above the clock

/**
 * @brief Paint the whole scene: sky, stars, arc, disc, treelines, cabin, smoke and weather.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 */
void scene_draw(GContext *ctx, GRect bounds, const Palette *pal);

/** @} */
