/**
 * @file layout.h
 * @brief Spatial map for the Ridgeline face (200x228).
 *
 * The top half is the drawn scene (see scene/scene.h), so the slots here all sit on the
 * mountain mass below it: the clock over the near ridge, the date under it, and a stats row
 * along the bottom. The chrome (battery, bluetooth, meridiem) rides in the sky above.
 *
 * @ingroup watchface-ridgeline
 */
#pragma once
#include <pebble.h>

#include "ui/engine/engine.h"

/**
 * @addtogroup watchface-ridgeline
 * @{
 */

// --- Zones ---
// the engine text-slot ids, one per live readout this face shows. there is no condition word:
// the scene itself says what the weather is doing, so only the temperature is spelled out
// the meridiem is not a zone: it hugs the clock's own right edge, and how wide the clock draws
// depends on the format, so it is measured and painted with the chrome instead
enum
{
    ZONE_TIME,
    ZONE_DATE,
    ZONE_WEATHER,
    ZONE_HR,
    ZONE_STEPS,
    ZONE_COUNT
};

// --- Text Slots ---
// the clock clears the near ridge's crest across the span it covers, so it lands on solid ground
#define SLOT_TIME GRect(4, 108, 192, 86)      // big clock over the mountains (Hand 72)
#define SLOT_TIME_SM GRect(4, 112, 192, 86)   // nudged down so the smaller tier keeps the baseline
#define SLOT_DATE GRect(4, 179, 192, 28)      // date under the clock (Hand 22)
// AM/PM goes where there is room for it, and that differs by layout.
//
// On the standard clock it sits beside the digits, AM to their left and PM to their right, so
// the half of the day shows in its position as well as its letters. Hand 72 leaves about 30px
// spare either side of a centred clock, which is enough for both.
//
// The two bigger layouts have no room at all: Hand 96 puts the digits within 12px of the bezel,
// and clock plus marker comes to more than the screen is wide. There the marker drops into the
// channel above the colon instead, which is empty because a colon is two dots on the middle line.
#define MERIDIEM_GAP 4         ///< Air between the digits and a marker beside them
#define MERIDIEM_BESIDE_W 34   ///< The box a beside-the-clock marker is pulled to the near end of
#define MERIDIEM_W 40          ///< The box an above-the-colon marker is centred in
#define MERIDIEM_TOP 129       ///< Its cap line on the standard clock, level with the digits'

// --- Date Top ---
// the middle layout: the date moves up onto the strip but the readouts stay where they are, so
// the clock grows into the band the date left behind rather than into the whole lower half. it
// takes the same Hand 96 as the big clock, just higher up and with less room under it
#define SLOT_TIME_DATETOP GRect(4, 105, 192, 100)
#define SLOT_TIME_DATETOP_SM GRect(4, 121, 192, 100)
// the same channel, on this layout's own digits
#define MERIDIEM_TOP_DATETOP 138

// --- Big Clock ---
// the layout for someone who wants the time and nothing else. the readouts go entirely, the
// date moves up onto the strip, and the clock takes everything they leave behind. it is the
// same scene underneath: only what is printed on it changes
// the land runs from the near ridge's lowest crest (y=126, see scene.c) to the bottom of the
// screen, so that band is the space the clock has to itself. these boxes put the digits' own
// middle on the middle of it: a text layer hangs its text from the top of its box, and Hand 96
// drops it 32px, so the box sits that far above where the digits should start
#define SLOT_TIME_BIG GRect(4, 113, 192, 100)    // the clock, filling the land (Hand 96)
#define SLOT_TIME_BIG_SM GRect(4, 129, 192, 100) // the .beats token drops a tier and keeps that middle

// the date is centred on the screen rather than on what is left of the strip, so it lines up
// with the clock under it. Hand 18 hangs 5px below its box, and the strip is 20 tall, so the box
// starts above the screen to land the lettering on the strip's own middle. it runs the full
// clear span between the battery and the status glyphs, so a long format has room before it
// has to drop a size
#define SLOT_DATE_TOP GRect(34, -2, 132, 20)
// the same channel again, on the big clock's digits
#define MERIDIEM_TOP_BIG 146

// --- Stats Row ---
// temp grows rightwards off its glyph, heart rate grows out from the screen's centre line, and
// steps grows leftwards off a right edge held 5px in from the bezel. all three share a y, so the
// numbers sit on one line however wide the readings get, and each glyph is placed off its own
// slot at draw time so moving a slot moves its mark with it
#define SLOT_TEMP GRect(19, 204, 47, 26)      // temperature, right of its thermometer (Hand 18)
// the heart's number is centred on x=112, not on the screen's 100. the glyph rides on its left,
// so offsetting the number by half the glyph-plus-gap is what puts the *pair* on the centre
// line. the offset is a constant, so the pair stays centred whatever the reading reads
#define SLOT_HR GRect(82, 204, 60, 26)        // heart rate, the pair centred on x=100 (Hand 18)
#define SLOT_STEPS GRect(148, 204, 47, 26)    // steps or distance, right edge 5px in from the bezel (Hand 18)

// a wide reading drops to Hand 16, and a text layer hangs its text from the top of the box, so
// the smaller font would otherwise float above the row and pull in from the right. each
// fallback box is nudged down and across by the difference, which lands the small text on the
// same baseline and the same right edge as the big
#define SLOT_TEMP_SM GRect(21, 206, 47, 26)
#define SLOT_HR_SM GRect(84, 206, 60, 26)
#define SLOT_STEPS_SM GRect(150, 206, 47, 26)

// --- Stats Row Anchors ---
#define STAT_GLYPH_GAP 5     ///< Air between a glyph and the number it labels
#define STAT_TEMP_GLYPH_X 4  ///< Left edge of the thermometer, the one glyph pinned to the bezel
#define STAT_BASELINE 224    ///< The line every glyph and digit in the row sits on

// --- Status Bar ---
// a dark strip across the top the chrome always sits on. without it the battery and bluetooth
// have to read against whatever the sky is doing, which on a white-paper palette means white
// on white. the strip costs 20px of sky and makes every theme legible up there
#define TOP_BAR_H 20

// --- Chrome Icons ---
#define BT_ICON GRect(185, 3, 14, 14)
#define QT_ICON GRect(167, 3, 14, 14)  // quiet time, just left of the bluetooth glyph

// --- Battery Gauge ---
// 23px wide so 5 bars + 4 gaps fill the interior with a symmetric 1px margin both sides
#define BATT_RECT GRect(4, 4, 23, 11)

/**
 * @brief Register fonts and prepare the scene. Call after the window exists and before
 * engine_init.
 *
 * @param window The main window.
 */
void ridgeline_setup(Window *window);

/**
 * @brief Fill the engine's slot list: the scene draw-slot plus the text readouts.
 *
 * @param out The slot array to fill.
 * @param max How many slots out can hold.
 * @param bounds The window's root bounds (for the full-window scene slot).
 * @return How many slots were written.
 */
uint8_t ridgeline_build(EngineSlot *out, uint8_t max, GRect bounds);

/**
 * @brief Re-read the theme and the daylight, and re-colour the zones for the palette that
 * comes out of it.
 *
 * Follow with engine_rebuild so the text-slots pick up the new colours.
 */
void ridgeline_apply_theme(void);

/**
 * @brief Re-check whether the sun has come up or gone down since the last tick, re-colouring
 * the zones when it has.
 *
 * The palette flips at sunrise and sunset, and the text colours ride on the palette, so the
 * minute tick has to notice. It only reports a change on the one minute it actually happens,
 * which is what keeps the caller from rebuilding the slots every minute.
 *
 * @return True when the palette flipped and the slots need rebuilding.
 */
bool ridgeline_daylight_changed(void);

/**
 * @brief Tear down the scene and the fonts.
 */
void ridgeline_teardown(void);

/** @} */
