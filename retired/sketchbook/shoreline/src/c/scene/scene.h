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
#if defined(PBL_ROUND)
#define SKY_BAND_Y 49   ///< Where the upper sky starts giving way to the lower
#else
#define SKY_BAND_Y 40   ///< Where the upper sky starts giving way to the lower
#endif
#if defined(PBL_ROUND)
#define SKY_BLEND_H 27  ///< How far it takes to get there, stippled so there is no hard seam
#else
#define SKY_BLEND_H 22  ///< How far it takes to get there, stippled so there is no hard seam
#endif
#if defined(PBL_ROUND)
#define HORIZON_Y 93    ///< Where the sea meets the sky, and the arc's baseline
#else
#define HORIZON_Y 76    ///< Where the sea meets the sky, and the arc's baseline
#endif
#define ARC_CX (PBL_DISPLAY_WIDTH / 2)  ///< Centre of the sun's arc across the sky
#if defined(PBL_ROUND)
#define ARC_RX 114       ///< How far the arc reaches either side of centre
#else
#define ARC_RX 88       ///< How far the arc reaches either side of centre
#endif
#if defined(PBL_ROUND)
#define ARC_RY 51       ///< How high the arc climbs above the horizon, keeping the disc clear of the bar
#else
#define ARC_RY 42       ///< How high the arc climbs above the horizon, keeping the disc clear of the bar
#endif
#define DISC_R 12       ///< Radius of the sun and moon

// the round boat, which is big enough to fly the temperature on a pennant
#define BOAT_MAST_H 25     ///< Mast height above the hull
#define BOAT_FLAG_W 39     ///< Four characters with margin: "120F" and "-12C" both sit clear of the edge
#define BOAT_FLAG_H 15
#define BOAT_HULL_HALF 16  ///< Half the hull's widest row, sized so the boat carries its pennant
#define BOAT_HULL_ROWS 8  ///< Deep enough to read as a hull rather than a waterline
// how far in from each edge the boat's run stops. it has to clear its own pennant, which flies
// off one side, so this is wider than the hull alone would need
#define BOAT_MARGIN PBL_IF_ROUND_ELSE(56, 20)

// the water's edge is a curve, not a rule. the tide slides the whole curve up and down between
// these two, and every column of water is measured off wherever the curve has got to at that x.
// the swing is worth more than the tide range is: a shore that sweeps across the beach reads as
// a coastline, and one that runs straight reads as a ruled line whatever the water is doing
#if defined(PBL_ROUND)
#define TIDE_LO_Y 131    ///< The curve's baseline at dead low, with the sea at its narrowest
#else
#define TIDE_LO_Y 107    ///< The curve's baseline at dead low, with the sea at its narrowest
#endif
#if defined(PBL_ROUND)
#define TIDE_HI_Y 150   ///< The curve's baseline at high water
#else
#define TIDE_HI_Y 123   ///< The curve's baseline at high water
#endif
#if defined(PBL_ROUND)
#define SHORE_SWING 13  ///< How far the curve wanders either side of its baseline
#else
#define SHORE_SWING 10  ///< How far the curve wanders either side of its baseline
#endif

// the dry sand starts here whatever the sea is doing, which is what gives the clock one home
// while the water comes and goes. it has to clear the curve at its lowest reach on a full tide
#define DRY_SAND_Y (TIDE_HI_Y + SHORE_SWING)

#if defined(PBL_ROUND)
#define HAZE_TOP 78     ///< Where the sea-haze wash starts thinning out
#else
#define HAZE_TOP 64     ///< Where the sea-haze wash starts thinning out
#endif
#if defined(PBL_ROUND)
#define HAZE_BOTTOM 148 ///< Where it stops, down on the water
#else
#define HAZE_BOTTOM 121 ///< Where it stops, down on the water
#endif

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
