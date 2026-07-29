/**
 * @file scene.h
 * @brief The drawn landscape: sky bands, the sun or moon on its arc, two mountain ridges, and
 * whatever the weather is adding on top.
 *
 * Ridgeline has no baked background. Everything above the clock is painted here from the time,
 * the sunrise and sunset the phone sends, and the current condition, which is what lets the
 * same face read as a clear morning, an overcast afternoon, or a snowy night.
 *
 * @ingroup watchface-ridgeline
 */
#pragma once
#include <pebble.h>

#include "theme/theme.h"

/**
 * @addtogroup watchface-ridgeline
 * @{
 */

// --- Scene geometry ---
#define SKY_BAND_Y 48   ///< Where the upper sky starts giving way to the lower
#define SKY_BLEND_H 24  ///< How far it takes to get there, stippled so there is no hard seam
#define HORIZON_Y 124  ///< The arc's baseline, and the floor the precipitation falls to
#define ARC_CX 100     ///< Centre of the sun's arc across the sky
#define ARC_RX 86      ///< How far the arc reaches either side of centre
#define ARC_RY 78      ///< How high the arc climbs above the horizon, kept under the status bar
#define DISC_R 12      ///< Radius of the sun and moon
#define FOG_TOP 82     ///< Where the fog wash starts thinning out
#define FOG_BOTTOM 126 ///< Where it stops, just above the clock

/**
 * @brief Whether it is after dark.
 *
 * Worked out from the sunrise and sunset the phone sends, falling back to a plain 06:00 to
 * 18:00 day when there is no reading yet. The condition token's "_NIGHT" marker is
 * deliberately not used: the arc has to agree with the palette, so both read the same clock.
 *
 * @return True once the sun is down.
 */
bool scene_night(void);

/**
 * @brief Allocate the ridge paths. Call once before the first draw.
 */
void scene_init(void);

/**
 * @brief Paint the whole scene: sky, stars, arc, disc, ridges, and the weather on top.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 */
void scene_draw(GContext *ctx, GRect bounds, const Palette *pal);

/**
 * @brief Free the ridge paths.
 */
void scene_deinit(void);

/** @} */
