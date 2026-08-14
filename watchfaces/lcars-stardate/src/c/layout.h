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
//
// the four ops zones are the pickable slots. two per column
// they carry no fixed reading. what they show comes from the catalog in ops/
// and their bar word and glyph follow the pick
//
// each one has a WIDE twin for a readout that carries no glyph. it reaches left
// over the space the glyph would have taken. left-aligned text sitting beside an
// empty icon box looks like a picture that failed to load
//
// ZONE_WEATHER and ZONE_COND are the composite left column. one pick fills both
// left rows. only one of the two left presentations is ever built so the slot
// count stays at ten
enum
{
    ZONE_TIME,
    ZONE_MERIDIEM,
    ZONE_DATE,
    ZONE_WEATHER,
    ZONE_COND,
    ZONE_LT,
    ZONE_LT_WIDE,
    ZONE_LB,
    ZONE_LB_WIDE,
    ZONE_RT,
    ZONE_RT_WIDE,
    ZONE_RB,
    ZONE_RB_WIDE,
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
// --- Ops slot geometry ---
// all four slots are the same shape so it gets described once here and stamped
// out per column and row. every offset is measured from the slot's bar origin
//
// the value box runs to the far edge of its column. catalog readings get long so
// every pixel of room here is one that keeps a reading on the full-size font
// instead of stepping it down
//
// dy is the nudge a smaller font needs. text sits at the top of its box so a step
// down has to push the box down by the cap height it lost. otherwise the reading
// floats above the line the other rows sit on. same trick the date banner uses
#define OPS_GLYPH_W 3   // gap from the bar's left edge to the glyph
#define OPS_TEXT_GAP 3  // gap from the glyph to the reading so the two do not crowd

// where a reading starts: past the glyph and its gap
#define OPS_TEXT_X(x) ((x) + OPS_GLYPH_W + 14 + OPS_TEXT_GAP)

// a value box is handed its column's right edge rather than a width. that way the
// gap above it can change without dragging the right margin along with it
#define OPS_BAR(x, y)               GRect((x), (y), 69, 11)
#define OPS_LABEL(x, y)             GRect((x) + 8, (y), 54, 11)
#define OPS_GLYPH(x, y)             GRect((x) + OPS_GLYPH_W, (y) + 13, 14, 14)
#define OPS_VALUE(x, y, r, dy)      GRect(OPS_TEXT_X(x), (y) + 11 + (dy), (r) - OPS_TEXT_X(x), 18)
#define OPS_VALUE_WIDE(x, y, r, dy) GRect((x) + OPS_GLYPH_W, (y) + 11 + (dy), \
                                          (r) - ((x) + OPS_GLYPH_W), 18)

// the two columns and the two rows they each hold
#define OPS_COL_L 52
#define OPS_COL_R 125
#define OPS_ROW_T 166
#define OPS_ROW_B 197

// how far right a reading may run. the right column goes to x=198 which is two
// off the screen edge. the left has to stop short of the right column's bar so it
// is narrower and steps its font down a little sooner
#define OPS_R_L 122
#define OPS_R_R 198

// the font step-down nudges in the order the Zone tiers try them
// Antonio 16 then 14 then 12
#define OPS_DY_FB 1
#define OPS_DY_FB2 3

// --- Coordinates ---
// right-aligned black text on the left rail blocks (Antonio 12)
#define COORD_TEXT_COLOR GColorBlack
#define SLOT_LAT GRect(0, 166, 43, 22)  // top block (coral)
#define SLOT_LON GRect(0, 203, 43, 22)  // bottom block (peach)

// --- Icons ---
// sized 1:1 to match bitmap
#define WX_ICON GRect(51, 176, 24, 24)      // centered above thermometer
// the weather glyphs are not all cut to the same margin. any whose art starts
// higher than this inside its box gets nudged down to it. that keeps the whole
// set clear of the bar overhead
#define WX_MIN_TOP 2
#define THERMO_ICON GRect(56, 204, 13, 17)  // centered below weather icon
#define BT_ICON GRect(30, 95, 14, 14)       // bluetooth status in the first left-rail block
#define QT_ICON GRect(15, 95, 14, 14)       // quiet-time status, just left of the bluetooth glyph

// --- LCARS Header Bars ---
// these get drawn rather than baked into the background art. each one is a pill
// an 8px rounded cap at either end with a square body between them
// a 1px notch of the black field sits between each cap and the body
// the label holder box is painted over the top afterwards
#define BAR_CAP_W 8
#define BAR_NOTCH 1

// the stardate bar spans the whole width so it gets its own rect. the four slot
// bars come from OPS_BAR. the label rides a pixel high inside its holder box and
// the spare row underneath is what squares that up
#define BAR_STARDATE GRect(52, 2, 142, 11)

// --- LCARS Bar Labels ---
// left-aligned in black holder boxes on colored bars
// the holder box matches its bar's height exactly so no sliver of bar shows under the label
#define LBL_STARDATE GRect(60, 2, 54, 11)

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
 * @brief Whether any drawn ops slot is showing the .beats reading.
 *
 * A beat is 86.4 seconds, so a slot showing one needs its own ticker rather than the minute
 * tick. The pick is independent of the clock's own time format, so this asks the slots.
 *
 * @return True if at least one slot on screen shows .beats.
 */
bool stardate_ops_shows_beats(void);

/**
 * @brief Re-apply the theme by swapping the frame bitmap.
 */
void stardate_apply_theme(void);

/**
 * @brief Tear down the frame, overlays, and fonts.
 */
void stardate_teardown(void);

/** @} */
