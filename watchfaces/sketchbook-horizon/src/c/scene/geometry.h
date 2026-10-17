/**
 * @file geometry.h
 * @brief Where a scene's sky, weather and ground sit, handed to the shared drawing code at runtime.
 *
 * The shared sky and weather code draws the same way for every scene, but not in the same places:
 * each scene puts its horizon somewhere else, so its arc, its clouds, its rain and its wash follow.
 * One scene is up at a time, so it sets itself as the active one and the shared code reads these
 * numbers back through sketchbook_scene_geometry.
 *
 * A scene fills one of these per platform, since a round screen wants different numbers from a
 * rectangle. The two that never vary (the arc is centred, every disc is the same size) are fixed
 * here for everyone rather than left to each scene.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "fx/fx.h"  // Puff, for the overcast band

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/// One leg of a lightning fork, a pair of points. A typedef rather than GPoint(*)[2] written out,
/// since GPoint is a function-like macro and the parenthesis beside it would be taken for a call.
typedef GPoint BoltLeg[2];

// the arc is centred on the screen and every scene's sun and moon are the same size, so these do
// not vary from one scene to the next
#define SKETCHBOOK_ARC_CX (PBL_DISPLAY_WIDTH / 2)  ///< The arc is centred on the screen
#define SKETCHBOOK_DISC_R 12                       ///< Sun and moon radius

/**
 * @brief The numbers the shared sky and weather code needs, all of a scene's in one place.
 *
 * Every y here is a pixel row on the current platform. The two arrays are the scene's own data:
 * the overcast band it hangs across its sky, and the lightning fork struck over it.
 *
 * A scene's values have to hold together or the weather draws wrong with nothing to flag it:
 * - cloud_y_min <= cloud_y_max, or the drifting cloud's clamp is inverted
 * - wash_top < wash_bottom, since the wash runs down the screen
 * - wash_top <= wash_wisp_hi_y <= wash_bottom, and the same for wash_wisp_lo_y, so each wisp sits
 *   inside the wash it gives an edge to
 * - precip_top + precip_h <= 228, so the fall stays on the screen
 */
typedef struct SceneGeometry
{
    // --- the sky ---
    int16_t sky_band_y;   ///< Where the upper sky starts giving way to the lower
    int16_t sky_blend_h;  ///< How far the stippled blend takes to get there
    int16_t arc_base_y;   ///< The baseline the disc rises off and sets into
    int16_t arc_rx;       ///< How far the arc reaches either side of centre
    int16_t arc_ry;       ///< How high it climbs above that baseline

    // --- clouds ---
    int16_t cloud_drop;   ///< How far under the disc a drifting cloud parks
    int16_t cloud_y_min;  ///< The highest it may park
    int16_t cloud_y_max;  ///< And the lowest
    uint8_t drift_lobe_a; ///< Lobe radius of the drifting cloud's near puff
    uint8_t drift_lobe_b; ///< And of its far one
    const Puff *overcast; ///< The overcast band
    uint8_t overcast_count;

    // --- what falls ---
    int16_t precip_top;   ///< Top of the band the fall is spread over
    int16_t precip_h;     ///< How deep that band runs

    // --- the wash: fog, sea haze or mist, depending on the scene ---
    int16_t wash_top;        ///< Where it starts thinning out
    int16_t wash_bottom;     ///< Where it stops
    int16_t wash_wisp_hi_y;  ///< The upper solid wisp through the densest part
    int16_t wash_wisp_lo_y;  ///< And the lower one

    // --- the lightning fork ---
    const BoltLeg *bolt_stroke;  ///< Each leg of the fork, as a pair of points
    uint8_t bolt_count;
} SceneGeometry;

/**
 * @brief The active scene's geometry, which the shared sky and weather code reads.
 *
 * The dispatcher sets this to whichever scene is up (see scene.c). It is never null once the face
 * is running, since a scene is always selected.
 *
 * @return The geometry of the scene being drawn.
 */
const SceneGeometry *sketchbook_scene_geometry(void);

/** @} */
