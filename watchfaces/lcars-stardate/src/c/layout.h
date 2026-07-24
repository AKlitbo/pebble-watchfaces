/**
 * @file layout.h
 * @brief Spatial map for the LCARS frame (200x228), kept in sync with the background image.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include <pebble.h>

#include "ui/engine/engine.h"

/**
 * @defgroup watchface-lcars LCARS Watchface
 * @brief The LCARS Stardate Emery watchface.
 * @{
 */

// --- Zone (text slot) ids ---
// face-owned slot vocabulary. layout.c declares a static Zone table indexed by these
// and the engine loops it to build the text layers
enum
{
    ZONE_TIME,
    ZONE_MERIDIEM,
    ZONE_DATE,
    ZONE_WEATHER,
    ZONE_COND,
    ZONE_HR,
    ZONE_STEPS,
    ZONE_LAT,
    ZONE_LON,
    ZONE_COUNT
};

// --- Text Slots ---
#define SLOT_TIME GRect(53, 92, 142, 74)       // centered time box (Antonio 62)
#define SLOT_BANNER GRect(53, 17, 142, 44)     // date below STARDATE bar (Antonio 36)
#define SLOT_BANNER_SM GRect(53, 20, 142, 43)  // wide date fallback vertically centered (Antonio 32)
#define SLOT_BANNER_XS GRect(53, 22, 142, 42)  // widest date fallback vertically centered (Antonio 28)
#define SLOT_MERIDIEM GRect(150, 95, 45, 14)   // AM/PM top-right of clock and right-aligned (Antonio 10)
#define SLOT_WEATHER GRect(72, 202, 49, 24)    // temp left-aligned next to thermometer
#define SLOT_COND GRect(77, 180, 42, 18)       // condition abbrev next to weather icon
#define SLOT_HR GRect(145, 177, 50, 18)        // heartrate value next to heart icon
#define SLOT_STEPS GRect(145, 208, 50, 18)     // steps value next to feet icon

// --- Coordinates ---
// right-aligned black text on the left rail blocks (Antonio 12)
#define COORD_TEXT_COLOR GColorBlack
#define SLOT_LAT GRect(0, 166, 43, 22)  // top block (coral)
#define SLOT_LON GRect(0, 203, 43, 22)  // bottom block (peach)

// --- Icons ---
// sized 1:1 to match bitmap
#define WX_ICON GRect(51, 176, 24, 24)      // centered above thermometer
#define THERMO_ICON GRect(56, 204, 13, 17)  // centered below weather icon
#define HR_ICON GRect(128, 179, 14, 14)
#define FEET_ICON GRect(128, 210, 14, 14)
#define BT_ICON GRect(30, 95, 14, 14)       // bluetooth status in the first left-rail block

// --- LCARS Bar Labels ---
// left-aligned in black holder boxes on colored bars

#define LBL_STARDATE GRect(60, 2, 54, 10)
#define LBL_WEATHER GRect(60, 166, 54, 10)
#define LBL_HR GRect(133, 166, 54, 10)
#define LBL_STEPS GRect(133, 197, 54, 10)

/**
 * @brief Register fonts, build the baked frame + overlays, and set the background.
 *
 * Call after the window exists and before engine_init, so the frame sits under the slots.
 *
 * @param window The main window.
 */
void stardate_setup(Window *window);

/**
 * @brief Fill the engine's slot list: the text readouts plus the overlays draw-slot.
 *
 * @param out The slot array to fill.
 * @param max How many slots out can hold.
 * @param bounds The window's root bounds (for the full-window overlays slot).
 * @return How many slots were written.
 */
uint8_t stardate_build(EngineSlot *out, uint8_t max, GRect bounds);

/**
 * @brief Re-apply the theme by swapping the frame bitmap.
 */
void stardate_apply_theme(void);

/**
 * @brief Tear down the frame, overlays, and fonts.
 */
void stardate_teardown(void);

/** @} */
