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
 * @ingroup watchface-ridgeline
 */
#pragma once

// --- the sky ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_SKY_BAND_Y 56   ///< Where the upper sky starts giving way to the lower
#else
#define SKETCHBOOK_SKY_BAND_Y 48   ///< Where the upper sky starts giving way to the lower
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_SKY_BLEND_H 28  ///< How far the stippled blend takes to get there
#else
#define SKETCHBOOK_SKY_BLEND_H 24  ///< How far the stippled blend takes to get there
#endif
// the baseline sits on the far crest rather than down behind the ridges, so the arc's ends tuck
// into the flanks the way shoreline's land on its sea horizon. run any lower and the ends leave
// the screen at the sides, which reads as a shallow fragment instead of a full sweep
#if defined(PBL_ROUND)
#define SKETCHBOOK_ARC_BASE_Y 107  ///< The baseline the disc rises off and sets into
#else
#define SKETCHBOOK_ARC_BASE_Y 124  ///< The baseline the disc rises off and sets into
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_ARC_RX 112     ///< How far the arc reaches either side of centre
#else
#define SKETCHBOOK_ARC_RX 86      ///< How far the arc reaches either side of centre
#endif
// a long shallow stroke, the same rise-to-reach as shoreline's
#if defined(PBL_ROUND)
#define SKETCHBOOK_ARC_RY 55      ///< How high it climbs above that baseline
#else
#define SKETCHBOOK_ARC_RY 78      ///< How high it climbs above that baseline
#endif

// --- clouds ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_CLOUD_DROP 22   ///< How far under the disc a drifting cloud parks
#else
#define SKETCHBOOK_CLOUD_DROP 20   ///< How far under the disc a drifting cloud parks
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_CLOUD_Y_MIN 62  ///< The highest it may park, keeping its lobes off the status bar
#else
#define SKETCHBOOK_CLOUD_Y_MIN 57  ///< The highest it may park, keeping its lobes off the status bar
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_CLOUD_Y_MAX 86  ///< And the lowest, keeping it clear of the scene below
#else
#define SKETCHBOOK_CLOUD_Y_MAX 82  ///< And the lowest, keeping it clear of the scene below
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_DRIFT_LOBE_A 10  ///< Lobe radius of the drifting cloud's near puff
#define SKETCHBOOK_DRIFT_LOBE_B 11  ///< And of its far one
#else
#define SKETCHBOOK_DRIFT_LOBE_A 9  ///< Lobe radius of the drifting cloud's near puff
#define SKETCHBOOK_DRIFT_LOBE_B 10  ///< And of its far one
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_OVERCAST_PUFFS { {18, 68, 33, 11}, {75, 64, 30, 12}, {135, 71, 34, 11}, {195, 65, 31, 12}, {238, 72, 28, 10} }
#else
#define SKETCHBOOK_OVERCAST_PUFFS { {14, 60, 26, 10}, {58, 56, 24, 11}, {104, 63, 27, 10}, {150, 57, 25, 11}, {192, 64, 22, 9} }
#endif

// --- what falls ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_PRECIP_TOP 40  ///< Top of the band the fall is spread over
#else
#define SKETCHBOOK_PRECIP_TOP 52  ///< Top of the band the fall is spread over
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_PRECIP_H 102      ///< How deep that band runs, stopping above the clock
#else
#define SKETCHBOOK_PRECIP_H 70      ///< How deep that band runs, stopping above the clock
#endif

// --- the wash: this face's fog banks over the ridges ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_WASH_TOP 88        ///< Where it starts thinning out
#else
#define SKETCHBOOK_WASH_TOP 82        ///< Where it starts thinning out
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_WASH_BOTTOM 142     ///< Where it stops
#else
#define SKETCHBOOK_WASH_BOTTOM 126     ///< Where it stops
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_WASH_WISP_HI_Y 122   ///< The upper solid wisp through the densest part
#else
#define SKETCHBOOK_WASH_WISP_HI_Y 112   ///< The upper solid wisp through the densest part
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_WASH_WISP_LO_Y 132   ///< And the lower one
#else
#define SKETCHBOOK_WASH_WISP_LO_Y 118   ///< And the lower one
#endif

// --- the lightning fork ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_BOLT_STROKE { {{151, 64}, {127, 96}}, {{127, 96}, {148, 93}}, {{148, 93}, {116, 124}} }
#else
#define SKETCHBOOK_BOLT_STROKE { {{116, 58}, {98, 86}}, {{98, 86}, {114, 84}}, {{114, 84}, {88, 120}} }
#endif
