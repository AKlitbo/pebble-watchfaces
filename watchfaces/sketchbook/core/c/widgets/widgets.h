/**
 * @file widgets.h
 * @brief The painted chrome: the battery gauge, the bluetooth glyph, and the three little
 * marks that label the stats row.
 *
 * All stateless draws the scene's overlay slot calls. The bluetooth glyph comes from the
 * shared icon cache and the engine owns the layer.
 *
 * Shared by the family, so it reads the face's own layout.h for where the chrome sits:
 * BATT_RECT, BT_ICON, QT_ICON, the MERIDIEM_* boxes and the stats-row anchors. Every face in
 * the family agrees on those. A face that moves one moves only its own chrome, which is the
 * point of them being there rather than here.
 *
 * @ingroup family-sketchbook
 */
#pragma once
#include <pebble.h>

#include "theme/theme.h"        // the face's Palette
#include "sketchbook/theme/battery.h"
#include "ui/fonts.h"  // FontId, for the clock font the marker is measured against
#include "ui/zone.h"   // Zone, for the boxes the stats row landed in

/**
 * @addtogroup family-sketchbook
 * @{
 */

/**
 * @brief Draw the battery gauge: an outline with five level-coloured segments.
 *
 * @param ctx The graphics context.
 * @param ink The colour to draw it in when the charge is healthy.
 * @param theme The theme setting value, which decides whether the low-charge warning colours apply.
 * @param level Battery charge level percentage.
 */
void widgets_draw_battery(GContext *ctx, GColor ink, uint8_t theme, int level);

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
 * @param connected True when the phone link is up.
 */
void widgets_draw_bt(GContext *ctx, bool connected);

/**
 * @brief Draw the second status strip, the one the date sits on.
 *
 * Only a round screen needs it, and only for a layout that moves the date up top. Everywhere else
 * this is a no-op, since the single strip already has the room.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 */
void widgets_draw_date_bar(GContext *ctx, GRect bounds);

/**
 * @brief Whether this watch can report a heart rate, so the stats row knows how many to place.
 *
 * False on hardware with no sensor and on a watch whose wearer has turned theirs off. Either way
 * the row drops to two readouts and re-centres rather than keeping a slot that can only ever say
 * "--". Asked per draw, since a sensor can come and go while the face is up.
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
 * The two zones come in rather than being read off the face's macros, because a round layout can
 * move that row: the standard one flanks the clock and the wider ones keep it along the bottom.
 * Taking the boxes the numbers actually drew in is what keeps each mark with its own number.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param weather The temperature's zone.
 * @param steps The steps zone.
 */
void widgets_draw_stat_glyphs(GContext *ctx, const Palette *pal, const Zone *weather, const Zone *steps);

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
 * @brief Draw the muted-speaker glyph that marks Quiet Time, left of the bluetooth glyph.
 *
 * The caller decides whether Quiet Time is on and whether the user asked to see it, so this
 * just paints.
 *
 * @param ctx The graphics context.
 * @param color The colour to paint it.
 */
void widgets_draw_qt(GContext *ctx, GColor color);

/**
 * @brief Draw the Quiet Time mark for either state, where the face bundles a pair for it.
 *
 * Bluetooth already works this way, and an always-filled slot is what stops the strip looking
 * lopsided half the time. Falls back to drawing only the muted mark where there is no pair.
 *
 * @param ctx The graphics context.
 * @param color The colour to paint it.
 * @param active True while Quiet Time is holding.
 */
void widgets_draw_qt_state(GContext *ctx, GColor color, bool active);

/** @} */
