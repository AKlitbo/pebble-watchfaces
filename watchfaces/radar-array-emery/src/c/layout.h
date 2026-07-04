/**
 * @file layout.h
 * @brief Spatial map for the radar-array frame (200x228), kept in sync with the baked background image.
 *
 * Live values fill the SLOT_* rects (engine text-slots), and the static labels
 * fill the LBL_* rects (drawn by widgets, so they can localize).
 *
 * @ingroup watchface-radar
 */
#pragma once
#include <pebble.h>

#include "ui/engine/engine.h"

/**
 * @defgroup watchface-radar Radar Array Watchface
 * @brief The Radar Array Emery watchface.
 * @{
 */

// --- Text Slots ---
#define SLOT_TIME GRect(10, 71, 180, 56)        // big clock centered in the scope (STM 40)
#define SLOT_MERIDIEM GRect(70, 111, 60, 16)    // AM/PM below the time and centered (STM 12)
#define SLOT_BANNER GRect(44, 168, 136, 22)     // date shifted right to clear the bluetooth cell (STM 18)
#define SLOT_BANNER_SM GRect(40, 168, 140, 20)  // wide date fallback (STM 17)
#define SLOT_COORD GRect(6, 15, 188, 16)        // lat/lon readout in the top band (STM 14)
#define SLOT_WEATHER GRect(6, 203, 64, 18)      // temp value (TEMP cell) (STM 18)
#define SLOT_HR GRect(69, 203, 62, 18)          // heartrate value (PULSE cell) (STM 18)
#define SLOT_STEPS GRect(130, 203, 62, 18)      // steps/distance value (RANGE cell) (STM 18)

// --- Colors ---
// the coordinate readout is green on the black field like the other readouts
#define COORD_TEXT_COLOR GColorGreen

// --- Static Labels ---
#define LBL_TEMP GRect(8, 192, 62, 11)     // TEMP cell label (STM 12)
#define LBL_PULSE GRect(69, 192, 62, 11)   // PULSE cell label (STM 12)
#define LBL_RANGE GRect(130, 192, 62, 11)  // RANGE cell label (STM 12)

// --- Battery Gauge ---
// 23px wide so 5 bars + 4 gaps fill the interior with a symmetric 1px margin both sides
#define BATT_RECT GRect(6, 6, 23, 11)

// --- Icons ---
// the 14px glyph centered on the baked amber cell at the left end of the date bar
#define BT_ICON GRect(28, 172, 14, 14)

/**
 * @brief Register fonts, build the baked frame + overlays, and apply the theme colours.
 *
 * Call after the window exists and before engine_init, so the frame sits under the slots.
 *
 * @param window The main window.
 */
void radar_setup(Window *window);

/**
 * @brief Fill the engine's slot list: the text readouts plus the overlays draw-slot.
 *
 * @param out The slot array to fill.
 * @param max How many slots out can hold.
 * @param bounds The window's root bounds (for the full-window overlays slot).
 * @return How many slots were written.
 */
uint8_t radar_build(EngineSlot *out, uint8_t max, GRect bounds);

/**
 * @brief Re-apply the theme: refresh the zone colours and swap the frame bitmap.
 *
 * Follow with engine_rebuild so the text-slots pick up the new colours.
 */
void radar_apply_theme(void);

/**
 * @brief Tear down the frame, overlays, and fonts.
 */
void radar_teardown(void);

/** @} */
