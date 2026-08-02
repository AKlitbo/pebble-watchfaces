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
 * @ingroup watchface-shoreline
 */
#pragma once

// --- the sky ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_SKY_BAND_Y 49   ///< Where the upper sky starts giving way to the lower
#else
#define SKETCHBOOK_SKY_BAND_Y 40   ///< Where the upper sky starts giving way to the lower
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_SKY_BLEND_H 27  ///< How far the stippled blend takes to get there
#else
#define SKETCHBOOK_SKY_BLEND_H 22  ///< How far the stippled blend takes to get there
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_ARC_BASE_Y 93  ///< The baseline the disc rises off and sets into
#else
#define SKETCHBOOK_ARC_BASE_Y 76  ///< The baseline the disc rises off and sets into
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_ARC_RX 114      ///< How far the arc reaches either side of centre
#else
#define SKETCHBOOK_ARC_RX 88      ///< How far the arc reaches either side of centre
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_ARC_RY 51      ///< How high it climbs above that baseline
#else
#define SKETCHBOOK_ARC_RY 42      ///< How high it climbs above that baseline
#endif

// --- clouds ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_CLOUD_DROP 22   ///< How far under the disc a drifting cloud parks
#else
#define SKETCHBOOK_CLOUD_DROP 18   ///< How far under the disc a drifting cloud parks
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_CLOUD_Y_MIN 66  ///< The highest it may park, keeping its lobes off the status bar
#else
#define SKETCHBOOK_CLOUD_Y_MIN 54  ///< The highest it may park, keeping its lobes off the status bar
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_CLOUD_Y_MAX 81  ///< And the lowest, keeping it clear of the scene below
#else
#define SKETCHBOOK_CLOUD_Y_MAX 66  ///< And the lowest, keeping it clear of the scene below
#endif
#define SKETCHBOOK_DRIFT_LOBE_A 8  ///< Lobe radius of the drifting cloud's near puff
#define SKETCHBOOK_DRIFT_LOBE_B 9  ///< And of its far one
#if defined(PBL_ROUND)
#define SKETCHBOOK_OVERCAST_PUFFS { {18, 68, 34, 11}, {70, 65, 32, 11}, {124, 72, 36, 11}, {178, 66, 33, 11}, {212, 73, 30, 10} }
#else
#define SKETCHBOOK_OVERCAST_PUFFS { {14, 56, 26, 9}, {58, 53, 24, 9}, {104, 59, 27, 9}, {150, 54, 25, 9}, {192, 60, 22, 8} }
#endif

// --- what falls ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_PRECIP_TOP 37  ///< Top of the band the fall is spread over
#else
#define SKETCHBOOK_PRECIP_TOP 30  ///< Top of the band the fall is spread over
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_PRECIP_H 115      ///< How deep that band runs, stopping above the clock
#else
#define SKETCHBOOK_PRECIP_H 94      ///< How deep that band runs, stopping above the clock
#endif

// --- the wash: this face's sea haze over the water ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_WASH_TOP 78        ///< Where it starts thinning out
#else
#define SKETCHBOOK_WASH_TOP 64        ///< Where it starts thinning out
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_WASH_BOTTOM 148     ///< Where it stops
#else
#define SKETCHBOOK_WASH_BOTTOM 121     ///< Where it stops
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_WASH_WISP_HI_Y 125   ///< The upper solid wisp through the densest part
#else
#define SKETCHBOOK_WASH_WISP_HI_Y 102   ///< The upper solid wisp through the densest part
#endif
#if defined(PBL_ROUND)
#define SKETCHBOOK_WASH_WISP_LO_Y 138   ///< And the lower one
#else
#define SKETCHBOOK_WASH_WISP_LO_Y 113   ///< And the lower one
#endif

// --- the lightning fork ---
#if defined(PBL_ROUND)
#define SKETCHBOOK_BOLT_STROKE { {{146, 76}, {128, 98}}, {{128, 98}, {144, 95}}, {{144, 95}, {120, 127}} }
#else
#define SKETCHBOOK_BOLT_STROKE { {{116, 62}, {98, 80}}, {{98, 80}, {114, 78}}, {{114, 78}, {90, 104}} }
#endif
