/**
 * @file letter.h
 * @brief Measure and letter a short line of text on a board in Hand 16, centred, with the font's
 * nudge.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief How wide a short line of text letters in Hand 16, measured in a 120 by 26 box.
 *
 * @param text The text.
 * @return Its width in pixels.
 */
int letter_width(const char *text);

/**
 * @brief Letter text in the middle of a board in Hand 16.
 *
 * @param ctx The graphics context.
 * @param text The text.
 * @param board The board to centre it on.
 * @param color The lettering's colour.
 */
void letter_board(GContext *ctx, const char *text, GRect board, GColor color);

/** @} */
