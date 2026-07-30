/**
 * @file widgets.h
 * @brief The painted chrome: the battery gauge, the bluetooth glyph, and the three little
 * marks that label the stats row.
 *
 * All stateless draws the scene's overlay slot calls. The bluetooth glyph comes from the
 * shared icon cache and the engine owns the layer.
 *
 * @ingroup watchface-ridgeline
 */
#pragma once
#include <pebble.h>

#include "theme/theme.h"
#include "ui/fonts.h"  // FontId, for the clock font the marker is measured against

/**
 * @addtogroup watchface-ridgeline
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
 * @brief Draw the stats row's marks: a thermometer, a heart, and a pair of tracks.
 *
 * The bundled glyphs are white masters, so each is recoloured to the palette on the way out
 * and reads in the same ink as the number beside it.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
void widgets_draw_stat_glyphs(GContext *ctx, const Palette *pal);

/**
 * @brief Draw the AM/PM marker beside the clock: AM on its left, PM on its right.
 *
 * Morning to the left and afternoon to the right reads like a day running left to right, so the
 * half of the day shows in the marker's position as well as its letters.
 *
 * The clock is centred, so where its edges land moves with the format and the time. It is
 * measured each paint to keep the marker against the digits rather than floating.
 *
 * Only fits where the clock leaves room beside it. Draws nothing on a 24-hour or .beats clock.
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
 * For a clock too wide to have anything beside it. A colon is two dots on the middle line, so
 * the channel between the digit pairs is clear above them and the marker sits in it. Where that
 * channel falls is measured rather than assumed: a wide hour like "08" pushes the colon somewhere
 * a narrow one like "10" does not, and a fixed column would put the marker through a digit.
 *
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

/** @} */
