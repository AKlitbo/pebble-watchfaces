/**
 * @file layout.h
 * @brief Puts the chosen desktop on screen: its baked frame, its fonts, and the engine slots
 * that draw the live readouts over it.
 *
 * Each desktop's frame is a baked background PNG holding all of its chrome. The C code paints
 * only the live readouts, the clock, the date, the weather, heart rate, and steps, plus the
 * weather glyph, the battery, and the Quiet Time and bluetooth status. Where each one goes is in
 * the desktop's layout table (desktop/desktop.h).
 *
 * @ingroup watchface-vscode
 */
#pragma once
#include <pebble.h>

#include "ui/engine/engine.h"
#include "desktop/desktop.h"

/**
 * @defgroup watchface-vscode VS Code Watchface
 * @brief The VS Code Emery watchface.
 * @{
 */

/**
 * @brief Load the chosen desktop's fonts and frame and apply its colours.
 *
 * Call after the window exists and before engine_init, so the frame sits under the slots.
 *
 * @param window The main window.
 */
void vscode_setup(Window *window);

/**
 * @brief Fill the engine's slot list: the text readouts plus the overlays draw-slot.
 *
 * @param out The slot array to fill.
 * @param max How many slots out can hold.
 * @param bounds The window's root bounds (for the full-window overlays slot).
 * @return How many slots were written.
 */
uint8_t vscode_build(EngineSlot *out, uint8_t max, GRect bounds);

/**
 * @brief Re-apply the Desktop and Theme settings: pick the layout table, paint the theme's
 * colours over VS Code's, swap the frame bitmap, and reload the fonts when the desktop changed.
 *
 * Follow with engine_rebuild, since the text-slots only pick up new fonts and colours when they
 * are made again.
 */
void vscode_apply_theme(void);

/**
 * @brief The layout the screen is drawn from, with the theme's colours already on it.
 *
 * @return The active layout.
 */
const DesktopLayout *vscode_layout(void);

/**
 * @brief Tear down the frame, overlays, and fonts.
 */
void vscode_teardown(void);

/** @} */
