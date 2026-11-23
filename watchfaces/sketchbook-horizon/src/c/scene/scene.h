/**
 * @file scene.h
 * @brief One face, six scenes, picked at runtime. The wearer chooses a scene in settings and the
 * face draws that one.
 *
 * Every scene is its own ground under a shared sky: ridgeline's mountains, shoreline's sea,
 * skyline's bridge, treeline's forest, and the landmarks, Mount Fuji and Giza. They all share the
 * sky, the weather and the chrome, and differ in what stands under it and where the clock and
 * readouts sit. Each scene is one row of the table in scene.c, and the layout engine runs whichever
 * row the Scene setting names.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "scene/frame.h"
#include "scene/geometry.h"

/**
 * @defgroup watchface-sketchbook Sketchbook Watchface
 * @brief A hand-drawn sun, moon and weather scene that follows the sky, in a scene the wearer picks.
 * @{
 */

/// The scenes, by the value the Scene setting saves. New ones only ever go on the end, since a
/// saved value has to keep meaning the same scene.
typedef enum
{
    SCENE_RIDGELINE = 0,
    SCENE_SHORELINE,
    SCENE_SKYLINE,
    SCENE_TREELINE,
    SCENE_FUJI,
    SCENE_GIZA,
    SCENE_COUNT
} SceneId;

/**
 * @brief One scene, as data: where its sky and weather sit, its ground colours, its own draws, and
 * the row its Clock Only clock sits on.
 *
 * The geometry is read by the shared sky and weather code, and the rest by the layout engine (see
 * layout/engine.h), both while this scene is up.
 *
 * The engine paints a frame in one order. The sky goes down first, then the clouds, then the
 * scene's ground, then the rain, the wash and the fork, and last the scene's front. The ground is
 * everything the weather falls in front of. The front is whatever stands in front of the weather,
 * such as the holder a round screen letters the temperature on, so the reading stays clear in
 * every condition.
 *
 * The ground row is passed as `const void *` because each scene has its own ground type with its
 * own fields. Every one leads with its GroundReadouts, which is all the engine reads from it, and
 * the scene's own draws cast back to the full row.
 */
typedef struct SceneDef
{
    const SceneGeometry *geometry;                                ///< Where its sky and weather sit
    const void *(*ground_for_theme)(uint8_t theme, bool night);  ///< Its ground row for the hour
    SceneDraw ground;      ///< Paint its ground and props, between the clouds and the weather
    SceneDraw front;       ///< Paint what stands in front of the weather, or NULL for nothing
    int16_t clock_only_y;  ///< Top of the Clock Only clock's box on the rectangle
} SceneDef;

/**
 * @brief The scene for a Scene setting value.
 *
 * A value past the last scene, from a damaged save, gets Ridgeline rather than a read off the end
 * of the table. So does a scene whose row was left out of the table.
 *
 * @param id The Scene setting's value.
 * @return The scene. Never NULL.
 */
const SceneDef *sketchbook_scene_def(uint8_t id);

/** @} */
