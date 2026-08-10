/**
 * @file text.h
 * @brief How this face writes a clock.
 *
 * The panels get their formatting through the gridlock accessors in engine/grid_shim.c, and those
 * answer from here, so the pennant and a panel can never disagree about whether it is half twelve
 * or 12:30.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

#include "ui/fonts.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

/**
 * @brief Draw a line centred in a box, on its caps rather than on its line box.
 *
 * Centring on what graphics_draw_text hands back puts the glyphs high, because a font's line box
 * carries descender room the digits never use. This measures from the cap height in the metrics
 * table instead, so a row of numbers sits where the eye expects it.
 *
 * @param ctx The graphics context.
 * @param text The line to draw.
 * @param font The font slot, which is also the metrics key.
 * @param box The area to centre in.
 */
void side_draw_centred(GContext *ctx, const char *text, FontId font, GRect box);

/**
 * @brief Whether the clock should read as 12 hour.
 *
 * Follows the time-format setting, falling back to the watch's own preference on System
 * Default. .beats has no hour to write so it reads as 24 hour here.
 *
 * @return True for 12 hour.
 */
bool side_is_12h(void);

/**
 * @brief Write an hour and minute in the user's format.
 *
 * @param out The destination buffer.
 * @param size Its size.
 * @param hour The hour, 0 to 23.
 * @param minute The minute, 0 to 59.
 */
void side_format_clock(char *out, size_t size, int hour, int minute);

/**
 * @brief Write just the hour in the user's format, as the pennant shows it.
 *
 * @param out The destination buffer.
 * @param size Its size.
 * @param hour The hour, 0 to 23.
 */
void side_format_hour(char *out, size_t size, int hour);

/** @} */
