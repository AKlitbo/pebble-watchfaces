/**
 * @file fx.h
 * @brief What the weather adds to a Sketchbook scene: clouds, falling precipitation, a wash over the
 * ground, and lightning.
 *
 * The layout engine reads the phone's condition into a SketchbookSky (see conditions.h) once a
 * frame and draws these around the scene's ground. Clouds go behind the ground, and the rain, the
 * wash and the bolts fall in front of it. Where each part lands comes from the scene's
 * SceneGeometry in scene/geometry.h.
 *
 * The wash is one thing with many names across the scenes, such as Ridgeline's fog, Shoreline's
 * sea haze, and Treeline's mist. It draws the same in every scene, so it is drawn once here and
 * each scene keeps its own word for it in its own prose.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "scene/frame.h"

/**
 * @addtogroup watchface-sketchbook
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
 * @param frame This frame's reading, whose disc a peeking cloud lands on.
 */
void sketchbook_fx_draw_clouds(GContext *ctx, const SceneFrame *frame);

/**
 * @brief Draw the weather in front of the scene: the rain or snow, then the wash, then the bolts.
 *
 * @param ctx The graphics context.
 * @param frame This frame's reading.
 */
void sketchbook_fx_draw_weather(GContext *ctx, const SceneFrame *frame);

/**
 * @brief Send a pair of birds across the sky, right to left under the arc, on a burst that brings
 * them. By day, with the sun out and nothing falling, only. They are off the screen at both ends of
 * the burst, so the still frame has an empty sky.
 *
 * @param ctx The graphics context.
 * @param frame This frame's reading.
 */
void sketchbook_fx_draw_birds(GContext *ctx, const SceneFrame *frame);

/**
 * @brief Whether a storm's lightning lights the sky on this frame of a burst.
 *
 * @param anim The phase.
 * @return True on a strike's frame. Never true when still.
 */
bool sketchbook_fx_flash(AnimPhase anim);

/**
 * @brief Light a copy of the palette for a strike's frame. The sky and cloud colours go one step
 * lighter, and so does every part of the scene painted in them, so the whole scene flashes.
 *
 * @param pal The copy to light, changed in place.
 */
void sketchbook_fx_flash_palette(CommonPalette *pal);

/** @} */
