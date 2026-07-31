/**
 * @file layout.h
 * @brief Spatial map for the Shoreline face (200x228).
 *
 * The top half is the drawn seascape (see scene/scene.h), so the slots here all sit on the dry
 * sand below it: the clock on the beach, the date under it, and a stats row along the bottom.
 * The chrome (battery, bluetooth, meridiem) rides in the sky above.
 *
 * Nothing here moves with the tide. The dry sand starts at the high-water mark whatever the
 * water is doing, which is what lets the clock keep one home while the sea comes and goes.
 *
 * @ingroup watchface-shoreline
 */
#pragma once
#include <pebble.h>

#include "ui/engine/engine.h"

/**
 * @addtogroup watchface-shoreline
 * @{
 */

// --- Zones ---
// the engine text-slot ids, one per live readout this face shows. there is no condition word:
// the scene itself says what the weather is doing, so only the temperature is spelled out
// the meridiem is not a zone: it hugs the clock's own edge, and how wide the clock draws depends
// on the format, so it is measured and painted with the chrome instead
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
// the clock sits on the dry sand, which starts at the high-water mark, so there is always a
// strip of clear beach above the digits however far the sea has come in
#define SLOT_TIME GRect(4, 108, 192, 86)      // clock on the beach (Hand 72)
#define SLOT_TIME_SM GRect(4, 112, 192, 86)   // nudged down so the smaller tier keeps the baseline
#define SLOT_DATE GRect(4, 179, 192, 28)      // date under the clock (Hand 22)
// AM/PM sits beside the digits here, AM left and PM right, so the half of the day shows in its
// position as well as its letters. the bigger layouts have no room out there and put it above
// the colon instead
#define MERIDIEM_GAP 4         ///< Air between the digits and a marker beside them
#define MERIDIEM_BESIDE_W 34   ///< Box a beside-the-clock marker is pulled to the near end of
#define MERIDIEM_W 40          ///< Box an above-the-colon marker is centred in
#define MERIDIEM_TOP 131       ///< 2px under the digits' cap line

// --- Date Top ---
// the date moves onto the strip but the readouts stay, so the clock only grows into the band the
// date left. same Hand 92 as the big clock, just lower and with less room under it
#define SLOT_TIME_DATETOP GRect(0, 109, 200, 100)     // digits land at y 140..200, 4px clear of the readouts
#define SLOT_TIME_DATETOP_SM GRect(0, 125, 200, 100)
#define MERIDIEM_TOP_DATETOP 140  ///< The colon channel, on this layout's digits

// --- Big Clock ---
// the readouts go entirely and the clock takes everything they leave. same scene underneath
//
// the clock centres on the dry sand, y=122 down, which is the high-water mark rather than
// wherever the water happens to be: centring on the live waterline would walk the clock up and
// down the screen twice a day
//
// full-screen width is the fit budget, not the position: the text centres either way, so the
// extra room only buys headroom against the tier dropping. the widest clock any format makes is
// 190px against 198, so the fallback never fires and the size never changes
#define SLOT_TIME_BIG GRect(0, 114, 200, 100)    // the clock, filling the beach (Hand 92)
#define SLOT_TIME_BIG_SM GRect(0, 130, 200, 100) // the tier this size never reaches

// centred on the screen rather than on what is left of the strip, so it lines up with the clock.
// Hand 18 hangs 5px below its box and the strip is 20 tall, so the box starts above the screen to
// land the lettering on the strip's middle. it spans the clear run between the battery and the
// status glyphs, which is room enough for a long format
#define SLOT_DATE_TOP GRect(34, -2, 132, 20)
#define MERIDIEM_TOP_BIG 146  ///< The colon channel, on the big clock's digits

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
 * @brief Register fonts and pick the starting palette. Call after the window exists and before
 * engine_init.
 *
 * @param window The main window.
 */
void shoreline_setup(Window *window);

/**
 * @brief Fill the engine's slot list: the scene draw-slot plus the text readouts.
 *
 * @param out The slot array to fill.
 * @param max How many slots out can hold.
 * @param bounds The window's root bounds (for the full-window scene slot).
 * @return How many slots were written.
 */
uint8_t shoreline_build(EngineSlot *out, uint8_t max, GRect bounds);

/**
 * @brief Re-read the theme and the daylight, and re-colour the zones for the palette that
 * comes out of it.
 *
 * Follow with engine_rebuild so the text-slots pick up the new colours.
 */
void shoreline_apply_theme(void);

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
bool shoreline_daylight_changed(void);

/**
 * @brief Tear down the fonts and the icon cache.
 */
void shoreline_teardown(void);

/** @} */
