/**
 * @file chrome.h
 * @brief Where the chrome and the stats row sit, the same for every scene.
 *
 * The battery gauge, the bluetooth and Quiet Time glyphs, the AM/PM marker, and the stats row all
 * land in the same place whichever scene is drawn, so the shared chrome code (widgets.c) and each
 * scene's own layout read these from here rather than each keeping its own copy. A scene's clock
 * and date move from one scene to the next, so those stay with the scene.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// --- Status Bar ---
// a dark strip across the top the chrome always sits on. without it the battery and bluetooth
// have to read against whatever the scene is doing, which on a white-paper palette means white
// on white. the strip costs 20px of sky and makes every theme legible up there
//
// a round screen is 64px wide at y=4 and does not reach 138 until y=20, so one strip cannot hold
// the icons and the date the way the rectangle does. it takes two: the icons drop to where there
// is room for them, and the date gets a second strip below with nearly twice the width
#if defined(PBL_ROUND)
#define TOP_BAR_H 24
#define TOP_BAR2_Y 28         ///< Top of the date strip, with a slice of sky showing between
#define TOP_BAR2_H 26
#else
#define TOP_BAR_H 20
#endif

// --- Chrome Icons ---
#if defined(PBL_ROUND)
// battery in the middle with a mark either side, so the group is 67 wide rather than 100 and the
// strip above the scene can be that much shallower
#define BT_ICON GRect(150, 6, 14, 14)   // bluetooth flanks right
#define QT_ICON GRect(94, 6, 18, 14)    // quiet time flanks left, 18 wide for the pair
#else
#define BT_ICON GRect(185, 3, 14, 14)
#define QT_ICON GRect(163, 3, 18, 14)  // quiet time, 18 wide for the pair, just left of the bluetooth glyph
#endif

// --- Battery Gauge ---
// 23px wide so 5 bars + 4 gaps fill the interior with a symmetric 1px margin both sides
#if defined(PBL_ROUND)
#define BATT_RECT GRect(118, 7, 23, 11)  // centred, with the two marks either side
#else
#define BATT_RECT GRect(4, 4, 23, 11)
#endif

// --- Meridiem Marker ---
#define MERIDIEM_GAP 4         ///< Air between the digits and a marker beside them
#define MERIDIEM_BESIDE_W 34   ///< Box a beside-the-clock marker is pulled to the near end of
#define MERIDIEM_W 40          ///< Box an above-the-colon marker is centred in

// --- Stats Row ---
// temp grows rightwards off its glyph, heart rate grows out from the screen's centre line, and
// steps grows leftwards off a right edge held 5px in from the bezel. all three share a y, so the
// numbers sit on one line however wide the readings get
#define SLOT_HR GRect(82, 204, 60, 26)        // heart rate, the pair centred on x=100 (Hand 18)
#define SLOT_STEPS GRect(148, 204, 47, 26)    // steps or distance, right edge 5px in from the bezel (Hand 18)
// a wide reading drops to Hand 16, and a text layer hangs its text from the top of the box, so
// each fallback box is nudged down and across to land the small text on the same baseline and
// right edge as the big
#define SLOT_HR_SM GRect(84, 206, 60, 26)
#define SLOT_STEPS_SM GRect(150, 206, 47, 26)

// --- Stats Row Anchors ---
#define STAT_GLYPH_GAP 5     ///< Air between a glyph and the number it labels
#define STAT_TEMP_GLYPH_X 4  ///< Left edge of the thermometer, the one glyph pinned to the bezel
#define STAT_BASELINE 224    ///< The line every glyph and digit in the row sits on
// a round layout can put the row at the sides or along the bottom, so a mark works its baseline
// out from the box its number draws in rather than from one fixed line. 20px below the big tier's
// box is where that tier's text lands
#define STAT_ROW_BASELINE(big) ((big).origin.y + 20)

/** @} */
