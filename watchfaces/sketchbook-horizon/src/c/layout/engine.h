/**
 * @file engine.h
 * @brief The layout every scene shares: the palette, the chrome, the slot list and the ticker
 * response, run from a small per-scene description.
 *
 * The scenes draw different landscapes but wear the same clock, date, readouts and chrome, picked
 * the same way from the same palette. That part lives here once. A scene hands over a SceneLayout:
 * its own slot table, the two things that genuinely move from scene to scene (where the clock sits
 * for each layout mode, and where the AM/PM marker lands), and the hooks to draw its landscape and
 * read its palette.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "ui/engine/engine.h"
#include "ui/zone.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// the engine text-slot ids, one per live readout a scene shows. the same for every scene: the
// clock, the date, the temperature, heart rate and steps. there is no condition word, since the
// scene itself says what the weather is doing
enum
{
    ZONE_TIME,
    ZONE_DATE,
    ZONE_WEATHER,
    ZONE_HR,
    ZONE_STEPS,
    ZONE_COUNT
};

/**
 * @brief One scene's layout, the parts that differ from the shared engine.
 *
 * The palette and the landscape come back as `const void *` because each scene has its own
 * palette type with its own extra fields. The engine only ever reads the shared block, so it casts
 * to CommonPalette, and the scene's own draw casts back to its full palette.
 */
typedef struct SceneLayout
{
    Zone *zones;                                            ///< The scene's ZONE_COUNT slot table
    const void *(*palette_for_theme)(uint8_t theme, bool night);  ///< The scene's palette for the hour
    void (*scene_draw)(GContext *ctx, GRect bounds, const void *pal);  ///< Paint the landscape
    void (*apply_layout)(void);                             ///< Move the zones for the current layout mode
    int (*meridiem_above_top)(void);                        ///< Colon-channel cap line for the current mode
    int meridiem_beside_top;                                ///< Cap line for a marker beside the digits
    void (*scene_init)(void);                               ///< Allocate the scene's paths, or NULL
    void (*scene_deinit)(void);                             ///< Free them, or NULL
} SceneLayout;

/**
 * @brief Register the fonts, prepare the scene, and theme it. Call after the window exists and
 * before engine_init.
 *
 * @param lay The scene's layout.
 * @param window The main window.
 */
void layout_setup(const SceneLayout *lay, Window *window);

/**
 * @brief Fill the engine's slots: the scene and chrome draw-slots plus the text readouts.
 *
 * @param lay The scene's layout.
 * @param out The slot array to fill.
 * @param max How many slots out can hold.
 * @param bounds The window's root bounds.
 * @return How many slots were written.
 */
uint8_t layout_build(const SceneLayout *lay, EngineSlot *out, uint8_t max, GRect bounds);

/**
 * @brief Re-read the theme and daylight and re-colour the zones.
 *
 * @param lay The scene's layout.
 */
void layout_apply_theme(const SceneLayout *lay);

/**
 * @brief Re-pick the palette for this tick, re-colouring the zones, and say whether it moved.
 *
 * @param lay The scene's layout.
 * @return True when the palette moved and the slots need rebuilding.
 */
bool layout_palette_changed(const SceneLayout *lay);

/**
 * @brief Tear down the scene and the fonts.
 *
 * @param lay The scene's layout.
 */
void layout_teardown(const SceneLayout *lay);

/** @} */
