/**
 * @file scene.h
 * @brief One face, four scenes, picked at runtime. The wearer chooses a scene in settings and the
 * face draws that one.
 *
 * Every scene is a self-contained drawing: ridgeline's mountains, shoreline's sea, skyline's
 * bridge, treeline's forest. They all share the sky, the weather and the chrome, and differ in what
 * stands under it and where the clock and readouts sit. Each scene is one row of the table in
 * scene.c, and the layout engine runs whichever row the Scene setting names.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

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
    SCENE_COUNT
} SceneId;

/**
 * @brief One scene, as data: where its sky and weather sit, its palette, its landscape, and the row
 * its Clock Only clock sits on.
 *
 * The geometry is read by the shared sky and weather code, and the rest by the layout engine (see
 * layout/engine.h), both while this scene is up.
 *
 * The palette and the landscape pass it as `const void *` because each scene has its own palette
 * type with its own extra fields. The engine only ever reads the shared block, so it casts to
 * CommonPalette, and the scene's own draw casts back to its full palette.
 */
typedef struct SceneDef
{
    const SceneGeometry *geometry;                                 ///< Where its sky and weather sit
    const void *(*palette_for_theme)(uint8_t theme, bool night);  ///< Its palette for the hour
    void (*draw)(GContext *ctx, GRect bounds, const void *pal);    ///< Paint the landscape
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
