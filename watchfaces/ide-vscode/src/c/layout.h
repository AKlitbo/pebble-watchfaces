/**
 * @file layout.h
 * @brief Spatial map for the VS Code frame (200x228), kept in sync with the baked background image.
 *
 * The menu bar, activity bar, editor tab, terminal panel, status bar, and the readout
 * glyphs are all baked into the background PNG. The C code paints only the live readouts:
 * the clock, the date, the terminal values and weather glyph, and the Quiet Time,
 * bluetooth, and battery status at the right end of the tab strip.
 *
 * @ingroup watchface-vscode
 */
#pragma once
#include <pebble.h>

#include "ui/engine/engine.h"

/**
 * @defgroup watchface-vscode VS Code Watchface
 * @brief The VS Code Emery watchface.
 * @{
 */

// the editor pane sits right of the 18px activity bar, below the 16px menu bar and the tab strip,
// and above the 20px status bar. the clock fills the upper editor with the date under it, and a
// baked terminal panel (y148..208) holds the weather, heart rate, and steps values

// --- Zones ---
// the engine text-slot ids. one per live readout this face shows
// the clock carries no separate meridiem slot and the face shows no coordinates
// so it omits the ZONE_MERIDIEM and ZONE_LAT/ZONE_LON that wider faces carry
enum
{
    ZONE_TIME,
    ZONE_DATE,
    ZONE_COND,
    ZONE_WEATHER,
    ZONE_HR,
    ZONE_STEPS,
    ZONE_COUNT
};

// --- Clock + date ---
// the widest time in Teko 82, 00:04, is 166px of ink, which leaves it 9px and 7px inside the
// editor. the rect is 172 wide so the 179px .beats reading overflows it and drops to Teko 78
// a font of size N puts its baseline N below the top of the rect, so both end at y104
#define SLOT_TIME GRect(23, 22, 172, 86)
#define SLOT_TIME_SMALL GRect(23, 26, 172, 82)
#define SLOT_DATE GRect(18, 110, 182, 22)

// --- Terminal readouts (2x2 grid under the prompt) ---
// left column: weather condition over temp. right column: heart rate over steps. each
// value sits just right of its glyph, and Share Tech Mono 14 inks y+4..y+14
#define SLOT_COND GRect(38, 176, 70, 18)
#define SLOT_TEMP GRect(38, 189, 70, 18)
#define SLOT_HR GRect(124, 176, 76, 18)
#define SLOT_STEPS GRect(124, 189, 76, 18)

// --- Weather glyph ---
// the condition icon is app-drawn (swapped per condition and tinted) left of the condition text,
// in the same 12px column as the baked thermometer under it
#define WX_ICON GRect(24, 179, 12, 12)

// --- Tab Strip Status ---
// the right end of the tab strip holds the battery, bluetooth, and Quiet Time, all app-drawn
// they pack leftwards from the battery cell at the right edge: the cell, the percentage,
// bluetooth, then Quiet Time, each STATUS_GAP from the next. anything hidden closes up, so
// only BATT_ICON's x is fixed and the rest keep just their row and size
#define BATT_ICON GRect(181, 24, 13, 8)
#define BATT_TEXT GRect(0, 19, 0, 18)
#define BT_ICON GRect(0, 22, 10, 10)
#define QUIET_ICON GRect(0, 24, 11, 8)
#define STATUS_GAP 4

/**
 * @brief Register fonts, build the baked frame + overlays, and apply the theme colours.
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
 * @brief Re-apply the theme: refresh the zone colours and swap the frame bitmap.
 *
 * Follow with engine_rebuild so the text-slots pick up the new colours.
 */
void vscode_apply_theme(void);

/**
 * @brief Tear down the frame, overlays, and fonts.
 */
void vscode_teardown(void);

/** @} */
