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
#define SKETCHBOOK_SKY_BAND_Y 48   ///< Where the upper sky starts giving way to the lower
#define SKETCHBOOK_SKY_BLEND_H 24  ///< How far the stippled blend takes to get there
#define SKETCHBOOK_ARC_BASE_Y 124  ///< The baseline the disc rises off and sets into
#define SKETCHBOOK_ARC_RX 86      ///< How far the arc reaches either side of centre
#define SKETCHBOOK_ARC_RY 78      ///< How high it climbs above that baseline

// --- clouds ---
#define SKETCHBOOK_CLOUD_DROP 20   ///< How far under the disc a drifting cloud parks
#define SKETCHBOOK_CLOUD_Y_MIN 57  ///< The highest it may park, keeping its lobes off the status bar
#define SKETCHBOOK_CLOUD_Y_MAX 82  ///< And the lowest, keeping it clear of the scene below
#define SKETCHBOOK_DRIFT_LOBE_A 9  ///< Lobe radius of the drifting cloud's near puff
#define SKETCHBOOK_DRIFT_LOBE_B 10  ///< And of its far one
#define SKETCHBOOK_OVERCAST_PUFFS { {14, 60, 26, 10}, {58, 56, 24, 11}, {104, 63, 27, 10}, {150, 57, 25, 11}, {192, 64, 22, 9} }

// --- what falls ---
#define SKETCHBOOK_PRECIP_TOP 52  ///< Top of the band the fall is spread over
#define SKETCHBOOK_PRECIP_H 70      ///< How deep that band runs, stopping above the clock

// --- the wash: this face's fog banks over the ridges ---
#define SKETCHBOOK_WASH_TOP 82        ///< Where it starts thinning out
#define SKETCHBOOK_WASH_BOTTOM 126     ///< Where it stops
#define SKETCHBOOK_WASH_WISP_HI_Y 112   ///< The upper solid wisp through the densest part
#define SKETCHBOOK_WASH_WISP_LO_Y 118   ///< And the lower one

// --- the lightning fork ---
#define SKETCHBOOK_BOLT_STROKE { {{116, 58}, {98, 86}}, {{98, 86}, {114, 84}}, {{114, 84}, {88, 120}} }
