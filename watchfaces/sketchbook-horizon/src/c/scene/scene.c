/**
 * @file scene.c
 * @brief Runs whichever scene the Scene setting names, and hands the shared drawing code that
 * scene's geometry.
 *
 * One scene is up at a time. Its geometry is cached here so the shared sky and weather code can
 * read it back without going through the settings on every pixel.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/scene.h"

#include "settings_schema.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

static const SceneDef *s_active;
static const SceneGeometry *s_geo;
// kept so a scene change can set the new scene up against the same window
static Window *s_window;

/** @brief The scene for an id, falling back to the first one for anything out of range. */
static const SceneDef *def_for(uint8_t scene)
{
    switch (scene)
    {
        case SCENE_RIDGELINE: return ridgeline_scene_def();
        case SCENE_SHORELINE: return shoreline_scene_def();
        case SCENE_SKYLINE:   return skyline_scene_def();
        case SCENE_TREELINE:  return treeline_scene_def();
        default:              return ridgeline_scene_def();
    }
}

/** @brief Point the dispatcher at a scene and cache its geometry for the shared code. */
static void select(const SceneDef *def)
{
    s_active = def;
    s_geo = def->geometry;
}

const SceneGeometry *sketchbook_scene_geometry(void)
{
    return s_geo;
}

void sketchbook_setup(Window *window)
{
    s_window = window;
    select(def_for(sketchbook_scene()));
    layout_setup(s_active->layout, window);
}

uint8_t sketchbook_build(EngineSlot *out, uint8_t max, GRect bounds)
{
    return layout_build(s_active->layout, out, max, bounds);
}

void sketchbook_apply_theme(void)
{
    layout_apply_theme(s_active->layout);
}

bool sketchbook_palette_changed(void)
{
    return layout_palette_changed(s_active->layout);
}

bool sketchbook_scene_reselect(void)
{
    const SceneDef *next = def_for(sketchbook_scene());
    if (next == s_active)
    {
        return false;
    }

    // the scenes allocate different paths and their fonts have to be reloaded, so the old one is
    // torn down whole before the new one is set up
    layout_teardown(s_active->layout);
    select(next);
    layout_setup(s_active->layout, s_window);
    return true;
}

void sketchbook_teardown(void)
{
    layout_teardown(s_active->layout);
}

/** @} */
