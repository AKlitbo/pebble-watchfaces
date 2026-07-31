/**
 * @file scene.h
 * @brief The drawn seascape: sky bands, the sun or moon on its arc and its glitter on the water,
 * the sea, a boat riding the tide, the surf line, and the beach the clock sits on.
 *
 * Shoreline has no baked background. Everything above the clock is painted here from the time,
 * the sunrise and sunset the phone sends, the tide, and the current condition, which is what lets
 * the same face read as a bright low-tide morning or a rough night at high water.
 *
 * @ingroup watchface-shoreline
 */
#pragma once
#include <pebble.h>

#include "theme/theme.h"

/**
 * @addtogroup watchface-shoreline
 * @{
 */

// --- Scene geometry ---
#define SKY_BAND_Y 40   ///< Where the upper sky starts giving way to the lower
#define SKY_BLEND_H 22  ///< How far it takes to get there, stippled so there is no hard seam
#define HORIZON_Y 76    ///< Where the sea meets the sky, and the arc's baseline
#define ARC_CX 100      ///< Centre of the sun's arc across the sky
#define ARC_RX 88       ///< How far the arc reaches either side of centre
#define ARC_RY 42       ///< How high the arc climbs above the horizon, keeping the disc clear of the bar
#define DISC_R 12       ///< Radius of the sun and moon

// the water's edge is a curve, not a rule. the tide slides the whole curve up and down between
// these two, and every column of water is measured off wherever the curve has got to at that x.
// the swing is worth more than the tide range is: a shore that sweeps across the beach reads as
// a coastline, and one that runs straight reads as a ruled line whatever the water is doing
#define TIDE_LO_Y 107    ///< The curve's baseline at dead low, with the sea at its narrowest
#define TIDE_HI_Y 123   ///< The curve's baseline at high water
#define SHORE_SWING 10  ///< How far the curve wanders either side of its baseline

// the dry sand starts here whatever the sea is doing, which is what gives the clock one home
// while the water comes and goes. it has to clear the curve at its lowest reach on a full tide
#define DRY_SAND_Y (TIDE_HI_Y + SHORE_SWING)

#define HAZE_TOP 64     ///< Where the sea-haze wash starts thinning out
#define HAZE_BOTTOM 121 ///< Where it stops, down on the water

/**
 * @brief Paint the whole scene: sky, stars, arc, disc, glitter, sea, waves, boat, surf, and
 * the weather on top.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 */
void scene_draw(GContext *ctx, GRect bounds, const Palette *pal);

/** @} */
