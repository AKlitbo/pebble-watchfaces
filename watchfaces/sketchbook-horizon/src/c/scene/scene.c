/**
 * @file scene.c
 * @brief Every scene in one table, each row at its own SceneId.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/scene.h"

#include "scene/ridgeline.h"
#include "scene/shoreline.h"
#include "scene/skyline.h"
#include "scene/treeline.h"
#include "theme/ridgeline.h"
#include "theme/shoreline.h"
#include "theme/skyline.h"
#include "theme/treeline.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// each row sits at its own SceneId, so the order they are written in does not matter
//
// Ridgeline's clock sits lower, filling the land under the ridges. Shoreline's centres on the dry
// sand at the high-water mark rather than wherever the water happens to be, since following the
// live waterline would walk the clock up and down the screen twice a day. Skyline's centres on
// the water, from where the reflections stop, and Treeline's on the clearing floor
static const SceneDef s_scenes[] = {
    [SCENE_RIDGELINE] = {
        .geometry = &ridgeline_geometry, .palette_for_theme = ridgeline_palette_for_theme,
        .draw = ridgeline_scene_draw, .clock_only_y = 119,
    },
    [SCENE_SHORELINE] = {
        .geometry = &shoreline_geometry, .palette_for_theme = shoreline_palette_for_theme,
        .draw = shoreline_scene_draw, .clock_only_y = 113,
    },
    [SCENE_SKYLINE] = {
        .geometry = &skyline_geometry, .palette_for_theme = skyline_palette_for_theme,
        .draw = skyline_scene_draw, .clock_only_y = 113,
    },
    [SCENE_TREELINE] = {
        .geometry = &treeline_geometry, .palette_for_theme = treeline_palette_for_theme,
        .draw = treeline_scene_draw, .clock_only_y = 113,
    },
};

// a missing last row makes the table short and fails here. a row left out in the middle comes up
// all zeros instead, and sketchbook_scene_def hands back Ridgeline for it rather than a null draw
_Static_assert(ARRAY_LENGTH(s_scenes) == SCENE_COUNT, "every scene needs a row");

const SceneDef *sketchbook_scene_def(uint8_t id)
{
    if (id >= SCENE_COUNT || !s_scenes[id].draw)
    {
        return &s_scenes[SCENE_RIDGELINE];
    }

    return &s_scenes[id];
}

/** @} */
