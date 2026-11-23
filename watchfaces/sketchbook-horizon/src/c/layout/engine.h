/**
 * @file engine.h
 * @brief The layout every scene shares: the palette, the chrome, the slot list and the ticker
 * response, run for whichever scene the Scene setting names.
 *
 * The scenes draw different landscapes but wear the same clock, date, readouts and chrome, picked
 * the same way from the same palette. That part lives here once, with the slots in zones.c, and so
 * does painting the sky and the weather around each scene's ground. A scene is one row of the
 * table in scene/scene.c: its geometry, its palette, its ground and front draws, and the row its
 * Clock Only clock sits on. The engine holds the scene that is up.
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

/// The scene's slot tag, so a burst's frame can repaint the scene alone
#define SKETCHBOOK_TAG_SCENE (1u << 0)

/// Every other slot's tag. A store change still repaints these, since it repaints every tag
#define SKETCHBOOK_TAG_REST (1u << 1)

/**
 * @brief Load the fonts, put up the scene the settings name, and theme it. Call once at startup,
 * after the window exists and before engine_init.
 *
 * @param window The main window.
 */
void sketchbook_setup(Window *window);

/**
 * @brief Fill the engine's slots: the scene and chrome draw-slots plus the text readouts.
 *
 * @param out The slot array to fill.
 * @param max How many slots out can hold.
 * @param bounds The window's root bounds.
 * @return How many slots were written.
 */
uint8_t sketchbook_build(EngineSlot *out, uint8_t max, GRect bounds);

/**
 * @brief Re-read the theme and daylight and re-colour the zones.
 */
void sketchbook_apply_theme(void);

/**
 * @brief Re-pick the palette for this tick, re-colouring the zones, and say whether the slots need
 * rebuilding.
 *
 * @return True when the palette moved, or a heart rate came or went since the row was laid out.
 */
bool sketchbook_palette_changed(void);

/**
 * @brief Re-read the Scene setting, and if it changed, put up the new scene and theme it.
 *
 * Every scene uses the same fonts and icons, so they stay loaded across the change. Follow a true
 * return with engine_rebuild.
 *
 * @return True when the scene changed, so the caller rebuilds the slots.
 */
bool sketchbook_scene_reselect(void);

/**
 * @brief How many frames a burst runs for the weather on screen. A storm runs longer, and a dry
 * overcast day on a scene marked still by day gets none, since nothing there would move.
 *
 * @return The burst's length in frames, or 0 for no burst.
 */
uint8_t sketchbook_burst_ticks(void);

/**
 * @brief Let go of the copy of a wet burst's still part, if one is held. Call before a rebuild, so
 * the heap is not holding it while the slots are made again. A burst that carries on paints in full
 * until it makes a fresh copy.
 */
void sketchbook_still_drop(void);

/**
 * @brief Free the fonts and the icons. Call once at exit.
 */
void sketchbook_teardown(void);

/** @} */
