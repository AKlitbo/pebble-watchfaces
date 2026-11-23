/**
 * @file shoreline.c
 * @brief The Shoreline scene's hooks for the shared layout engine: its palette, its landscape, and
 * the row its Clock Only clock sits on. The slots are in zones.c and the chrome in engine.c.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/scene.h"

#include "scene/shoreline.h"
#include "theme/shoreline.h"
#include "layout/engine.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/** @brief The scene's palette for the hour, as the engine's opaque pointer. */
static const void *palette_for_theme(uint8_t theme, bool night)
{
    return shoreline_palette_for_theme(theme, night);
}

/** @brief Paint the landscape, handing the scene back its own palette type. */
static void scene_draw(GContext *ctx, GRect bounds, const void *pal)
{
    shoreline_scene_draw(ctx, bounds, pal);
}

static const SceneLayout s_layout = {
    .palette_for_theme = palette_for_theme,
    .scene_draw = scene_draw,
    // the Clock Only clock centres on the dry sand at the high-water mark rather than wherever
    // the water happens to be. centring on the live waterline would walk the clock up and down
    // the screen twice a day
    .clock_only_y = 113,
};

const SceneDef *shoreline_scene_def(void)
{
    static SceneDef def = {.layout = &s_layout};
    def.geometry = shoreline_geometry();
    return &def;
}

/** @} */
