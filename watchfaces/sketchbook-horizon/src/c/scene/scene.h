/**
 * @file scene.h
 * @brief One face, four scenes, picked at runtime. The wearer chooses a scene in settings and the
 * face draws that one.
 *
 * Every scene is a self-contained drawing: ridgeline's mountains, shoreline's sea, skyline's
 * bridge, treeline's forest. They all share the sky, the weather and the chrome, and differ in
 * what stands under it and where the clock and readouts sit. A scene hands over one SceneDef, and
 * this dispatcher runs whichever the Scene setting names.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "ui/engine/engine.h"
#include "scene/geometry.h"
#include "layout/engine.h"

/**
 * @defgroup watchface-sketchbook Sketchbook Watchface
 * @brief A hand-drawn sun, moon and weather scene that follows the sky, in a scene the wearer picks.
 * @{
 */

/// The scenes, in the Scene picker's option order.
typedef enum
{
    SCENE_RIDGELINE = 0,
    SCENE_SHORELINE,
    SCENE_SKYLINE,
    SCENE_TREELINE,
    SCENE_COUNT
} SceneId;

/**
 * @brief One scene, as data: where its sky and weather sit, and how it lays out.
 *
 * The geometry is read by the shared sky and weather code, and the layout by the shared layout
 * engine (see layout/engine.h), both while this scene is up. A scene is two tables and the handful
 * of hooks the layout engine calls back into, nothing more.
 */
typedef struct SceneDef
{
    const SceneGeometry *geometry;  ///< Where this scene's sky and weather sit
    const SceneLayout *layout;      ///< Its slots, and the hooks the layout engine runs
} SceneDef;

// each scene assembles its own, in its own file
const SceneDef *ridgeline_scene_def(void);
const SceneDef *shoreline_scene_def(void);
const SceneDef *skyline_scene_def(void);
const SceneDef *treeline_scene_def(void);

/**
 * @brief Point the face at the scene the settings name, then set it up. Call once at startup, after
 * the window exists and before engine_init.
 *
 * @param window The main window.
 */
void sketchbook_setup(Window *window);

/**
 * @brief Fill the engine's slots from the active scene.
 *
 * @param out The slot array to fill.
 * @param max How many slots out can hold.
 * @param bounds The window's root bounds.
 * @return How many slots were written.
 */
uint8_t sketchbook_build(EngineSlot *out, uint8_t max, GRect bounds);

/**
 * @brief Re-read the theme and daylight and re-colour the active scene's zones.
 */
void sketchbook_apply_theme(void);

/**
 * @brief Re-pick the active scene's palette for this tick, and say whether it moved.
 *
 * @return True when the palette moved and the slots need rebuilding.
 */
bool sketchbook_palette_changed(void);

/**
 * @brief Re-read the Scene setting, and if it changed, tear down the old scene and set up the new.
 *
 * The scenes allocate different paths and the fonts have to be reloaded, so a scene change is a
 * teardown and a fresh setup rather than a repaint. Follow a true return with engine_rebuild.
 *
 * @return True when the scene changed, so the caller rebuilds the slots.
 */
bool sketchbook_scene_reselect(void);

/**
 * @brief Tear down the active scene.
 */
void sketchbook_teardown(void);

/** @} */
