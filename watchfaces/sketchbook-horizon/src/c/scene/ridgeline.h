/**
 * @file ridgeline.h
 * @brief The Ridgeline scene: sky bands, the sun or moon on its arc, two mountain ridges, and
 * whatever the weather is adding on top.
 *
 * Ridgeline has no baked background. Everything above the clock is painted from the time, the
 * sunrise and sunset the phone sends, and the current condition, which is what lets the same scene
 * read as a clear morning, an overcast afternoon, or a snowy night.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "theme/ridgeline.h"
#include "scene/geometry.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/** @brief Allocate the ridge paths. Call once before the first draw. */
void ridgeline_scene_init(void);

/**
 * @brief Paint the whole scene: sky, stars, arc, disc, ridges, and the weather on top.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 */
void ridgeline_scene_draw(GContext *ctx, GRect bounds, const RidgelinePalette *pal);

/** @brief Free the ridge paths. */
void ridgeline_scene_deinit(void);

/** @brief Where this scene's sky and weather sit, for the shared drawing code. */
const SceneGeometry *ridgeline_geometry(void);

/** @} */
