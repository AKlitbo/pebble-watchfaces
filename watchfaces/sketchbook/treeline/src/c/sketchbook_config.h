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
 * @ingroup watchface-treeline
 */
#pragma once

// --- the sky ---
#define SKETCHBOOK_SKY_BAND_Y 46   ///< Where the upper sky starts giving way to the lower
#define SKETCHBOOK_SKY_BLEND_H 24  ///< How far the stippled blend takes to get there
#define SKETCHBOOK_ARC_BASE_Y 108  ///< The baseline the disc rises off and sets into
#define SKETCHBOOK_ARC_RX 86      ///< How far the arc reaches either side of centre
#define SKETCHBOOK_ARC_RY 72      ///< How high it climbs above that baseline

// --- clouds ---
#define SKETCHBOOK_CLOUD_DROP 18   ///< How far under the disc a drifting cloud parks
#define SKETCHBOOK_CLOUD_Y_MIN 54  ///< The highest it may park, keeping its lobes off the status bar
#define SKETCHBOOK_CLOUD_Y_MAX 66  ///< And the lowest, keeping it clear of the scene below
#define SKETCHBOOK_DRIFT_LOBE_A 8  ///< Lobe radius of the drifting cloud's near puff
#define SKETCHBOOK_DRIFT_LOBE_B 9  ///< And of its far one
#define SKETCHBOOK_OVERCAST_PUFFS { {14, 56, 26, 9}, {58, 53, 24, 9}, {104, 59, 27, 9}, {150, 54, 25, 9}, {192, 60, 22, 8} }

// --- what falls ---
#define SKETCHBOOK_PRECIP_TOP 30  ///< Top of the band the fall is spread over
#define SKETCHBOOK_PRECIP_H 94      ///< How deep that band runs, stopping above the clock

// --- the wash: this face's mist through the trees ---
#define SKETCHBOOK_WASH_TOP 81        ///< Where it starts thinning out
#define SKETCHBOOK_WASH_BOTTOM 129     ///< Where it stops
#define SKETCHBOOK_WASH_WISP_HI_Y 101   ///< The upper solid wisp through the densest part
#define SKETCHBOOK_WASH_WISP_LO_Y 111   ///< And the lower one

// --- the lightning fork ---
#define SKETCHBOOK_BOLT_STROKE { {{116, 62}, {98, 80}}, {{98, 80}, {114, 78}}, {{114, 78}, {90, 104}} }
