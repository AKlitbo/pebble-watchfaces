/**
 * @file fx.h
 * @brief What the weather adds to a Sketchbook scene: clouds, falling precipitation, a wash over the
 * ground, and a lightning fork.
 *
 * The face reads the phone's condition into a SketchbookSky (see conditions.h) and draws these in its
 * own z-order, since only it knows where its landscape sits: clouds go behind the ground, rain
 * falls in front of it. Where each part lands comes from the face's sketchbook_config.h.
 *
 * The wash is one thing with three names across the family: Ridgeline's fog, Shoreline's sea
 * haze, Treeline's mist. It draws the same either way, so it is drawn once here and each face
 * keeps its own word for it in its own prose.
 *
 * @ingroup family-sketchbook
 */
#pragma once
#include <pebble.h>

#include "sketchbook/fx/conditions.h"
#include "theme/theme.h"  // the face's Palette

/**
 * @addtogroup family-sketchbook
 * @{
 */

/** @brief One puff of a cloud: where its base sits, how wide it is, and how big its lobes are. */
typedef struct
{
    int16_t x;      ///< Bottom-centre
    int16_t y;      ///< Base line
    uint8_t half_w; ///< Half the base width
    uint8_t lobe;   ///< Radius of the outer lobes
} Puff;

/**
 * @brief Draw the sky's clouds. One cloud parks over the disc, a band sits across the sky.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param sky What the condition is doing.
 * @param disc Where the sun or moon is, so a peeking cloud lands on it.
 */
void sketchbook_fx_draw_clouds(GContext *ctx, const Palette *pal, const SketchbookSky *sky, GPoint disc);

/**
 * @brief Draw the falling precipitation as a curtain over the scene.
 *
 * Particle positions come from the minute rather than a random number, so the scene shifts
 * once a minute on the tick that is already happening and never costs a timer.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param sky What the condition is doing.
 * @param minute The clock minute, which shuffles the particles.
 */
void sketchbook_fx_draw_precip(GContext *ctx, const Palette *pal, const SketchbookSky *sky, int minute);

/**
 * @brief Draw the wash lying over the ground: the face's fog, haze or mist.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
void sketchbook_fx_draw_wash(GContext *ctx, const Palette *pal);

/**
 * @brief Draw a lightning fork out of the cloud.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
void sketchbook_fx_draw_bolt(GContext *ctx, const Palette *pal);

/** @} */
