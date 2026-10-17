/**
 * @file sketchbook_config.h
 * @brief Where this face's scene sits, for the shared Sketchbook drawing code.
 *
 * The family draws the sky, the weather and the chrome the same way for every face, but not in
 * the same places: each one puts its horizon somewhere else, so its clouds, its rain and its
 * wash follow. Those numbers are here, and families/line/c/config.h refuses to compile without
 * them.
 *
 * These are the face's own tuning. Changing one moves this face and nothing else.
 *
 * @ingroup watchface-skyline
 */
#pragma once

// --- the sky ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_SKY_BAND_Y 56  ///< Where the upper sky starts giving way to the lower
#else
#define SKETCHBOOK_SKY_BAND_Y 46   ///< Where the upper sky starts giving way to the lower
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_SKY_BLEND_H 28  ///< How far the stippled blend takes to get there
#else
#define SKETCHBOOK_SKY_BLEND_H 24  ///< How far the stippled blend takes to get there
#endif
// the baseline is the far bank, which the round build drops with the rest of the scene. the
// arc keeps the family's long shallow rise rather than the rectangle's steeper one
#if defined(PBL_ROUND)
#define SKETCHBOOK_ARC_BASE_Y 100  ///< The baseline the disc rises off and sets into
#else
#define SKETCHBOOK_ARC_BASE_Y 82  ///< The baseline the disc rises off and sets into
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_ARC_RX 112  ///< How far the arc reaches either side of centre
#else
#define SKETCHBOOK_ARC_RX 86      ///< How far the arc reaches either side of centre
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_ARC_RY 55  ///< How high it climbs above that baseline
#else
#define SKETCHBOOK_ARC_RY 54      ///< How high it climbs above that baseline
#endif

// --- clouds ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_CLOUD_DROP 22  ///< How far under the disc a drifting cloud parks
#else
#define SKETCHBOOK_CLOUD_DROP 18   ///< How far under the disc a drifting cloud parks
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_CLOUD_Y_MIN 52  ///< The highest it may park, keeping its lobes off the status bar
#else
#define SKETCHBOOK_CLOUD_Y_MIN 42  ///< The highest it may park, keeping its lobes off the status bar
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_CLOUD_Y_MAX 78  ///< And the lowest, keeping it clear of the scene below
#else
#define SKETCHBOOK_CLOUD_Y_MAX 54  ///< And the lowest, keeping it clear of the scene below
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_DRIFT_LOBE_A 10  ///< Lobe radius of the drifting cloud's near puff
#define SKETCHBOOK_DRIFT_LOBE_B 11  ///< And of its far one
#else
#define SKETCHBOOK_DRIFT_LOBE_A 8  ///< Lobe radius of the drifting cloud's near puff
#define SKETCHBOOK_DRIFT_LOBE_B 9  ///< And of its far one
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_OVERCAST_PUFFS { {18, 56, 33, 11}, {75, 52, 30, 12}, {135, 59, 34, 11}, {195, 53, 31, 12}, {238, 60, 28, 10} }
#else
#define SKETCHBOOK_OVERCAST_PUFFS { {14, 46, 26, 9}, {58, 43, 24, 9}, {104, 49, 27, 9}, {150, 44, 25, 9}, {192, 50, 22, 8} }
#endif

// --- what falls ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_PRECIP_TOP 42  ///< Top of the band the fall is spread over
#else
#define SKETCHBOOK_PRECIP_TOP 34  ///< Top of the band the fall is spread over
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_PRECIP_H 105  ///< How deep that band runs, stopping above the clock
#else
#define SKETCHBOOK_PRECIP_H 78      ///< How deep that band runs, stopping above the clock
#endif

// --- the wash: this face's fog off the river ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_WASH_TOP 82  ///< Where it starts thinning out
#else
#define SKETCHBOOK_WASH_TOP 66        ///< Where it starts thinning out
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_WASH_BOTTOM 152  ///< Where it stops
#else
#define SKETCHBOOK_WASH_BOTTOM 124     ///< Where it stops
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_WASH_WISP_HI_Y 104  ///< The upper solid wisp through the densest part
#else
#define SKETCHBOOK_WASH_WISP_HI_Y 84   ///< The upper solid wisp through the densest part
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_WASH_WISP_LO_Y 128  ///< And the lower one
#else
#define SKETCHBOOK_WASH_WISP_LO_Y 104   ///< And the lower one
#endif

// --- the lightning fork ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_BOLT_STROKE { {{151, 64}, {127, 88}}, {{127, 88}, {148, 86}}, {{148, 86}, {116, 114}} }
#else
#define SKETCHBOOK_BOLT_STROKE { {{116, 54}, {98, 72}}, {{98, 72}, {114, 70}}, {{114, 70}, {90, 94}} }
#endif
