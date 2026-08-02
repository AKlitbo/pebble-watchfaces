/**
 * @file layout.h
 * @brief Spatial map for the Treeline face (200x228).
 *
 * The top half is the drawn forest (see scene/scene.h), so the slots here all sit on the clearing
 * floor below it: the clock on the ground, the date under it, and a stats row along the bottom.
 * The chrome (battery, bluetooth, meridiem) rides in the sky above.
 *
 * The clearing floor starts at a fixed row, so the clock keeps one home whatever the weather is
 * doing above it.
 *
 * @ingroup watchface-treeline
 */
#pragma once
#include <pebble.h>

#include "ui/engine/engine.h"

/**
 * @addtogroup watchface-treeline
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
// the clock sits on the clearing floor, which starts above the digits whatever the trees are
// doing, so there is always a strip of clear ground between the two
//
// a round screen narrows fastest at the bottom, so the clock takes the bigger size and the date
// sits under it with the whole width still to itself. the temperature comes off this stack
// entirely and goes into the scene, because a third line down here does not fit
#if defined(PBL_ROUND)
#define SLOT_TIME GRect(20, 136, 220, 100)    // clock on the clearing (Hand 92 here, not 72)
#define SLOT_TIME_SM GRect(20, 152, 220, 100) // nudged down so the smaller tier keeps the baseline
#define SLOT_DATE GRect(20, 225, 220, 28)     // date under the clock (Hand 22)
#else
#define SLOT_TIME GRect(4, 108, 192, 86)      // clock on the clearing (Hand 72)
#define SLOT_TIME_SM GRect(4, 112, 192, 86)   // nudged down so the smaller tier keeps the baseline
#define SLOT_DATE GRect(4, 179, 192, 28)      // date under the clock (Hand 22)
#endif
// AM/PM sits beside the digits here, AM left and PM right, so the half of the day shows in its
// position as well as its letters. the bigger layouts have no room out there and put it above
// the colon instead
#define MERIDIEM_GAP 4         ///< Air between the digits and a marker beside them
#define MERIDIEM_BESIDE_W 34   ///< Box a beside-the-clock marker is pulled to the near end of
#define MERIDIEM_W 40          ///< Box an above-the-colon marker is centred in
// the round clock is Hand 92, and at the widest time a marker beside it lands hard against the
// bezel, so there it takes the colon channel like the bigger layouts do
#define MERIDIEM_TOP PBL_IF_ROUND_ELSE(163, 131)  ///< 2px under the digits' cap line

// --- Date Top ---
// the date moves onto the strip but the readouts stay, so the clock only grows into the band the
// date left. same Hand 92 as the big clock, just lower and with less room under it
#if defined(PBL_ROUND)
#define SLOT_TIME_DATETOP GRect(20, 99, 220, 100)
#define SLOT_TIME_DATETOP_SM GRect(20, 115, 220, 100)
#else
#define SLOT_TIME_DATETOP GRect(0, 109, 200, 100)     // digits land at y 140..200, 4px clear of the readouts
#define SLOT_TIME_DATETOP_SM GRect(0, 125, 200, 100)
#endif
#define MERIDIEM_TOP_DATETOP PBL_IF_ROUND_ELSE(130, 140)  ///< The colon channel, on this layout's digits

// --- Big Clock ---
// the readouts go entirely and the clock takes everything they leave. same scene underneath
//
// the clock centres on the clearing floor, y=120 down, which is where the ground starts
//
// full-screen width is the fit budget, not the position: the text centres either way, so the
// extra room only buys headroom against the tier dropping. the widest clock any format makes is
// 190px against 198, so the fallback never fires and the size never changes
#if defined(PBL_ROUND)
#define SLOT_TIME_BIG GRect(20, 117, 220, 100)   // the clock, filling the clearing (Hand 92)
#define SLOT_TIME_BIG_SM GRect(20, 133, 220, 100) // the tier this size never reaches
#else
#define SLOT_TIME_BIG GRect(0, 113, 200, 100)   // the clock, filling the clearing (Hand 92)
#define SLOT_TIME_BIG_SM GRect(0, 129, 200, 100) // the tier this size never reaches
#endif

// centred on the screen rather than on what is left of the strip, so it lines up with the clock.
// Hand 18 hangs 5px below its box and the strip is 20 tall, so the box starts above the screen to
// land the lettering on the strip's middle. it spans the clear run between the battery and the
// status glyphs, which is room enough for a long format
#if defined(PBL_ROUND)
#define SLOT_DATE_TOP GRect(45, 29, 170, 20)  // on the second strip, where the screen is 170 wide
#else
#define SLOT_DATE_TOP GRect(34, -2, 132, 20)
#endif
#define MERIDIEM_TOP_BIG PBL_IF_ROUND_ELSE(149, 145)  ///< The colon channel, on the big clock's digits

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

// --- Stats Row, No Heart Rate ---
// gabbro has no sensor, and an emery whose sensor cannot get a reading is in the same place, so
// the row drops to two and re-centres rather than leaving a permanent "--" in the middle.
//
// both survivors take the treatment the heart rate had: text centred in its box with the glyph
// riding in front, which is the only one of the three that was already placed as a *pair*. so
// the two pairs sit either side of the centre line and stay centred whatever they read
//
// there is no round version. gabbro has no sensor, its steps went with the room they wanted, and
// its temperature is painted into the scene, so no row is left down here to re-centre
#if !defined(PBL_ROUND)
#define SLOT_TEMP_PAIR GRect(35, 204, 60, 26)
#define SLOT_STEPS_PAIR GRect(125, 204, 60, 26)
#define SLOT_TEMP_PAIR_SM GRect(37, 206, 60, 26)
#define SLOT_STEPS_PAIR_SM GRect(127, 206, 60, 26)
#endif

// --- Stats Row Anchors ---
#define STAT_GLYPH_GAP 5     ///< Air between a glyph and the number it labels
#define STAT_TEMP_GLYPH_X 4  ///< Left edge of the thermometer, the one glyph pinned to the bezel
#define STAT_BASELINE 224    ///< The line every glyph and digit in the row sits on
// a round layout can put the row at the sides or along the bottom, so a mark works its baseline
// out from the box its number draws in rather than from one fixed line. 20px below the big tier's
// box is where that tier's text lands, and the small tier's box is nudged to match
#define STAT_ROW_BASELINE(big) ((big).origin.y + 20)

// --- Status Bar ---
// a dark strip across the top the chrome always sits on. without it the battery and bluetooth
// have to read against whatever the sky is doing, which on a white-paper palette means white
// on white. the strip costs 20px of sky and makes every theme legible up there
#if defined(PBL_ROUND)
// a round screen is 64px wide at y=4, so one strip cannot hold the icons and the date the way the
// rectangle does. the three marks cluster in the middle where the circle is widest soonest, and
// the date gets a second strip below on the layouts that move it up there
#define TOP_BAR_H 24
#define TOP_BAR2_Y 28         ///< Top of the date strip, with a slice of sky showing between
#define TOP_BAR2_H 26
#else
#define TOP_BAR_H 20
#endif

// --- Chrome Icons ---
#if defined(PBL_ROUND)
// battery in the middle with a mark either side, so the group is 67 wide rather than 100
#define BT_ICON GRect(150, 6, 14, 14)   // bluetooth flanks right
#define QT_ICON GRect(94, 6, 18, 14)    // quiet time flanks left, 18 wide for the pair
#else
#define BT_ICON GRect(185, 3, 14, 14)
#define QT_ICON GRect(167, 3, 14, 14)  // quiet time, just left of the bluetooth glyph
#endif

// --- Battery Gauge ---
// 23px wide so 5 bars + 4 gaps fill the interior with a symmetric 1px margin both sides
#if defined(PBL_ROUND)
#define BATT_RECT GRect(118, 7, 23, 11)  // centred, with the two marks either side
#else
#define BATT_RECT GRect(4, 4, 23, 11)
#endif

/**
 * @brief Register fonts and pick the starting palette. Call after the window exists and before
 * engine_init.
 *
 * @param window The main window.
 */
void treeline_setup(Window *window);

/**
 * @brief Fill the engine's slot list: the scene draw-slot plus the text readouts.
 *
 * @param out The slot array to fill.
 * @param max How many slots out can hold.
 * @param bounds The window's root bounds (for the full-window scene slot).
 * @return How many slots were written.
 */
uint8_t treeline_build(EngineSlot *out, uint8_t max, GRect bounds);

/**
 * @brief Re-read the theme and the daylight, and re-colour the zones for the palette that
 * comes out of it.
 *
 * Follow with engine_rebuild so the text-slots pick up the new colours.
 */
void treeline_apply_theme(void);

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
bool treeline_daylight_changed(void);

/**
 * @brief Tear down the fonts and the icon cache.
 */
void treeline_teardown(void);

/** @} */
