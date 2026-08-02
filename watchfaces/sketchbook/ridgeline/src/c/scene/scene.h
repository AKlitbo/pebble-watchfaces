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

// --- Trail sign ---
// a round screen has no room to stack a date and a reading under the clock, so the temperature
// comes off that stack and onto a signpost planted in the saddle right of the main peak. the
// board stands against the sky where it always has something to read against, and the post runs
// down into the slope below the crest
#if defined(PBL_ROUND)
#define SIGN_X 178         ///< The post's centre line
#define SIGN_BOARD_Y 108   ///< Top of the board
#define SIGN_BOARD_H 17    ///< How tall the board is
#define SIGN_BASE_Y 146    ///< Where the post meets the slope
#define SIGN_PAD 6         ///< Air either side of the reading, inside the board

#endif

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
