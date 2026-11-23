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
#include "scene/landmarks.h"
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
//
// a landmark stands on Skyline's horizon, so it shares Skyline's sky, stars and weather and its
// clock row. one on land paints its ground with Ridgeline's colours and one on water with Skyline's
//
// every scene strikes its own bolts, clear of its own tall features. a landmark shares Skyline's
// geometry but not its bolts
//
// a scene's front is what a round screen draws in front of the weather, the temperature's holder
// or the mast that carries it. the rectangle has none
static const SceneDef s_scenes[] = {
    [SCENE_RIDGELINE] = {
        .geometry = &ridgeline_geometry, .ground_for_theme = ridgeline_ground_for_theme,
        .ground = ridgeline_draw_ground, .front = PBL_IF_ROUND_ELSE(ridgeline_draw_front, NULL),
        .clock_only_y = 119,
        .bolts = &ridgeline_bolts,
    },
    [SCENE_SHORELINE] = {
        .geometry = &shoreline_geometry, .ground_for_theme = shoreline_ground_for_theme,
        .ground = shoreline_draw_ground, .front = PBL_IF_ROUND_ELSE(shoreline_draw_front, NULL),
        .clock_only_y = 113,
        .bolts = &shoreline_bolts,
    },
    [SCENE_SKYLINE] = {
        .geometry = &skyline_geometry, .ground_for_theme = skyline_ground_for_theme,
        .ground = skyline_draw_ground, .front = PBL_IF_ROUND_ELSE(skyline_draw_front, NULL),
        .clock_only_y = 113,
        .bolts = &skyline_bolts,
    },
    [SCENE_TREELINE] = {
        .geometry = &treeline_geometry, .ground_for_theme = treeline_ground_for_theme,
        .ground = treeline_draw_ground, .front = PBL_IF_ROUND_ELSE(treeline_draw_front, NULL),
        .clock_only_y = 113,
        .bolts = &treeline_bolts,
    },
    [SCENE_FUJI] = {
        .geometry = &skyline_geometry, .ground_for_theme = ridgeline_ground_for_theme,
        .ground = fuji_draw_ground, .front = PBL_IF_ROUND_ELSE(fuji_draw_front, NULL),
        .clock_only_y = 113,
        .bolts = &fuji_bolts,
    },
    [SCENE_GIZA] = {
        .geometry = &skyline_geometry, .ground_for_theme = ridgeline_ground_for_theme,
        .ground = giza_draw_ground, .front = PBL_IF_ROUND_ELSE(giza_draw_front, NULL),
        .clock_only_y = 113,
        .bolts = &giza_bolts,
    },
    [SCENE_HALF_DOME] = {
        .geometry = &skyline_geometry, .ground_for_theme = ridgeline_ground_for_theme,
        .ground = half_dome_draw_ground, .front = PBL_IF_ROUND_ELSE(half_dome_draw_front, NULL),
        .clock_only_y = 113,
        .bolts = &half_dome_bolts,
    },
    [SCENE_ULURU] = {
        .geometry = &skyline_geometry, .ground_for_theme = ridgeline_ground_for_theme,
        .ground = uluru_draw_ground, .front = PBL_IF_ROUND_ELSE(uluru_draw_front, NULL),
        .clock_only_y = 113,
        .bolts = &uluru_bolts,
    },
    [SCENE_NIAGARA] = {
        .geometry = &skyline_geometry, .ground_for_theme = skyline_ground_for_theme,
        .ground = niagara_draw_ground, .front = PBL_IF_ROUND_ELSE(niagara_draw_front, NULL),
        .clock_only_y = 113,
        .bolts = &niagara_bolts,
    },
    [SCENE_TABLE_MOUNTAIN] = {
        .geometry = &skyline_geometry, .ground_for_theme = skyline_ground_for_theme,
        .ground = table_mountain_draw_ground, .front = PBL_IF_ROUND_ELSE(table_mountain_draw_front, NULL),
        .clock_only_y = 113,
        .bolts = &table_mountain_bolts,
    },
    [SCENE_MONUMENT_VALLEY] = {
        .geometry = &skyline_geometry, .ground_for_theme = ridgeline_ground_for_theme,
        .ground = monument_valley_draw_ground, .front = PBL_IF_ROUND_ELSE(monument_valley_draw_front, NULL),
        .clock_only_y = 113,
        .bolts = &monument_valley_bolts,
    },
};

// a missing last row makes the table short and fails here. a row left out in the middle comes up
// all zeros instead, and sketchbook_scene_def hands back Ridgeline for it rather than a null ground
_Static_assert(ARRAY_LENGTH(s_scenes) == SCENE_COUNT, "every scene needs a row");

const SceneDef *sketchbook_scene_def(uint8_t id)
{
    if (id >= SCENE_COUNT)
    {
        return &s_scenes[SCENE_RIDGELINE];
    }

    // a row with no ground or no bolts would draw nothing or strike nowhere, so the whole scene
    // gives way to Ridgeline. that shows on every screen rather than only in a storm
    if (!s_scenes[id].ground || !s_scenes[id].bolts)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "scene %d needs its ground and its bolts in scene.c", id);
        return &s_scenes[SCENE_RIDGELINE];
    }

    return &s_scenes[id];
}

/** @} */
