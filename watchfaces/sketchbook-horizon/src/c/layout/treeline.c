/**
 * @file treeline.c
 * @brief The Treeline scene's hooks for the shared layout engine: its palette, its landscape, and
 * the row its Clock Only clock sits on. The slots are in zones.c and the chrome in engine.c.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/scene.h"

#include "scene/treeline.h"
#include "theme/treeline.h"
#include "layout/engine.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/** @brief The scene's palette for the hour, as the engine's opaque pointer. */
static const void *palette_for_theme(uint8_t theme, bool night)
{
    return treeline_palette_for_theme(theme, night);
}

/** @brief Paint the landscape, handing the scene back its own palette type. */
static void scene_draw(GContext *ctx, GRect bounds, const void *pal)
{
    treeline_scene_draw(ctx, bounds, pal);
}

static const SceneLayout s_layout = {
    .palette_for_theme = palette_for_theme,
    .scene_draw = scene_draw,
    // the Clock Only clock centres on the clearing floor, from where the ground starts
    .clock_only_y = 113,
};

const SceneDef *treeline_scene_def(void)
{
    static SceneDef def = {.layout = &s_layout};
    def.geometry = treeline_geometry();
    return &def;
}

/** @} */
