/**
 * @file ridgeline.c
 * @brief The Ridgeline scene's hooks for the shared layout engine: its palette, its landscape, and
 * the row its Clock Only clock sits on. The slots are in zones.c and the chrome in engine.c.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/scene.h"

#include "scene/ridgeline.h"
#include "theme/ridgeline.h"
#include "layout/engine.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/** @brief The scene's palette for the hour, as the engine's opaque pointer. */
static const void *palette_for_theme(uint8_t theme, bool night)
{
    return ridgeline_palette_for_theme(theme, night);
}

/** @brief Paint the landscape, handing the scene back its own palette type. */
static void scene_draw(GContext *ctx, GRect bounds, const void *pal)
{
    ridgeline_scene_draw(ctx, bounds, pal);
}

static const SceneLayout s_layout = {
    .palette_for_theme = palette_for_theme,
    .scene_draw = scene_draw,
    // the clock sits lower here, filling the land under the ridges
    .clock_only_y = 119,
    .scene_init = ridgeline_scene_init,
    .scene_deinit = ridgeline_scene_deinit,
};

const SceneDef *ridgeline_scene_def(void)
{
    static SceneDef def = {.layout = &s_layout};
    def.geometry = ridgeline_geometry();
    return &def;
}

/** @} */
