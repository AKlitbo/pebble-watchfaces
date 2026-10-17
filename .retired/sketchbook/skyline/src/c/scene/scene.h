/**
 * @file scene.h
 * @brief The drawn crossing: sky bands, the sun or moon on its arc, a city standing back, a
 * suspension bridge over the river, and the lamps that come on at dusk.
 *
 * Skyline has no baked background. Everything above the clock is painted here from the time,
 * the sunrise and sunset the phone sends, and the current condition, which is what lets the
 * same bridge read as a flat daytime silhouette or a string of lights on black water.
 *
 * @ingroup watchface-skyline
 */
#pragma once
#include <pebble.h>

#include "theme/theme.h"

/**
 * @addtogroup watchface-skyline
 * @{
 */

// --- Scene geometry ---
// the round screen is 30% wider and only 14% taller, so the crossing is not a straight scale of
// the rectangle: it spreads to the new width and drops far enough that the water still has the
// clock's whole band under it
#if defined(PBL_ROUND)
#define CITY_BASE 132    ///< The far bank, which the city stands on and the river starts at
#define WATER_TOP 132    ///< Same row: everything below the far bank is water
#define DECK_Y 124       ///< The bridge deck, crossing in front of the bank and over the river
#define TOWER_TOP 56     ///< How high the two bridge towers reach
#define PIER_DROP 14     ///< How far each tower carries on below the deck, into the river
#define SCENE_BOTTOM 162 ///< The last row the reflections reach, matching the rest of the family
// --- Channel buoy ---
// the marker floating mid-river below the crossing, carrying the reading on its side the way a
// real channel buoy carries its number
#define BUOY_TOP 143     ///< Top of the body, clear of the deck so it floats rather than hangs
#define BUOY_H 17        ///< How tall the body is
#define BUOY_MAST 7      ///< How far the topmark stands above it
#else
#define CITY_BASE 108    ///< The far bank, which the city stands on and the river starts at
#define WATER_TOP 108    ///< Same row: everything below the far bank is water
#define DECK_Y 102       ///< The bridge deck, crossing in front of the bank and over the river
#define TOWER_TOP 46     ///< How high the two bridge towers reach
#define PIER_DROP 10     ///< How far each tower carries on below the deck, into the river
#define SCENE_BOTTOM 133 ///< The last row the reflections reach, matching the rest of the family
#endif

/**
 * @brief Paint the whole scene: sky, stars, arc, disc, city, bridge, lamps and the water.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 */
void scene_draw(GContext *ctx, GRect bounds, const Palette *pal);

/** @} */
