/**
 * @file widgets.h
 * @brief The painted chrome: the battery gauge, the bluetooth glyph, and the three little
 * marks that label the stats row.
 *
 * All stateless draws the scene's overlay slot calls. The bluetooth glyph comes from the
 * shared icon cache and the engine owns the layer.
 *
 * Where the chrome sits comes from chrome.h: BATT_RECT, BT_ICON, QT_ICON, the MERIDIEM_* boxes
 * and the stats-row anchors. Every scene puts its chrome in the same place, so those live there
 * once and the scene layouts read the same ones.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "theme/palette.h"
#include "ui/fonts.h"  // FontId, for the clock font the marker is measured against
#include "ui/zone.h"   // Zone, for the boxes the stats row landed in

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Draw the battery gauge: an outline with five level-coloured segments.
 *
 * The outline goes in the bar's ink, and the segments warn in the palette's battery colours as the
 * charge runs down.
 *
 * @param ctx The graphics context.
 * @param pal The palette on screen.
 * @param level Battery charge level percentage.
 */
void widgets_draw_battery(GContext *ctx, const CommonPalette *pal, int level);

/**
 * @brief Paint the dark strip the chrome sits on, across the top of the screen.
 *
 * The scene underneath can be anything from white paper to a black night, so the chrome gets
 * its own ground rather than having to read against all of it.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 */
void widgets_draw_top_bar(GContext *ctx, GRect bounds);

/**
 * @brief Draw the bluetooth glyph for the link state (connected vs slashed).
 *
 * @param ctx The graphics context.
 * @param color The colour to paint it.
 * @param connected True when the phone link is up.
 */
void widgets_draw_bt(GContext *ctx, GColor color, bool connected);

// a round screen has no stats row, so these two are the rectangle's alone
#if !defined(PBL_ROUND)
/**
 * @brief Whether this watch can report a heart rate, so the stats row knows how many to place.
 *
 * False on hardware with no sensor and on a watch whose wearer has turned theirs off. Either way
 * the row drops to two readouts and re-centres rather than keeping a slot that can only ever say
 * "--". The layout asks at each rebuild, and the clock's tick rebuilds when the answer changes,
 * since a sensor can come and go while the face is up.
 *
 * @return True when a reading is obtainable.
 */
bool sketchbook_has_hr(void);

/**
 * @brief Draw the stats row's marks: a thermometer, a heart, and a pair of tracks.
 *
 * The bundled glyphs are white masters, so each is recoloured to the palette on the way out
 * and reads in the same ink as the number beside it.
 *
 * The two zones come in rather than being read off the chrome macros, because with no heart rate
 * the layout moves the temperature and steps into centred boxes. Taking the boxes the numbers
 * actually drew in is what keeps each mark with its own number.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param weather The temperature's zone.
 * @param steps The steps zone.
 * @param has_hr Whether the row was laid out with a heart rate, so the marks match the numbers.
 */
void widgets_draw_stat_glyphs(GContext *ctx, const CommonPalette *pal, const Zone *weather, const Zone *steps,
    bool has_hr);
#endif

/**
 * @brief Draw the AM/PM marker beside the clock: AM on its left, PM on its right.
 *
 * Reading the day left to right puts the half of the day in the marker's position as well as its
 * letters. The clock is centred, so its edges move with the format and are measured each paint to
 * keep the marker against the digits. Only fits where the clock leaves room beside it, and draws
 * nothing on a 24-hour or .beats clock.
 *
 * @param ctx The graphics context.
 * @param color The colour to paint it.
 * @param clock_slot The box the clock is drawn in.
 * @param clock_font The font the clock is drawn in, for measuring its width.
 * @param top Where the marker's own cap line sits.
 */
void widgets_draw_meridiem_beside(GContext *ctx, GColor color, GRect clock_slot, FontId clock_font, int top);

/**
 * @brief Draw the AM/PM marker in the gap above the clock's colon.
 *
 * For a clock too wide to have anything beside it. A colon is two dots on the middle line, so the
 * channel between the digit pairs is clear above them. Where it falls is measured, not assumed: a
 * wide hour like "08" moves the colon and a fixed column would put the marker through a digit.
 * Draws nothing on a 24-hour or .beats clock.
 *
 * @param ctx The graphics context.
 * @param color The colour to paint it.
 * @param clock_slot The box the clock is drawn in, for finding its centred left edge.
 * @param clock_font The font the clock is drawn in, for measuring where the colon lands.
 * @param top Where the marker's own cap line sits.
 */
void widgets_draw_meridiem_above(GContext *ctx, GColor color, GRect clock_slot, FontId clock_font, int top);

/**
 * @brief Draw the Quiet Time mark for either state.
 *
 * Bluetooth already works this way, and an always-filled slot is what stops the strip looking
 * lopsided half the time.
 *
 * @param ctx The graphics context.
 * @param color The colour to paint it.
 * @param active True while Quiet Time is holding.
 */
void widgets_draw_qt_state(GContext *ctx, GColor color, bool active);

/** @} */
