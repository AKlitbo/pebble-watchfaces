/**
 * @file measure.h
 * @brief Remembers how big a piece of text came out, so a repaint that measures the same text in
 * the same font skips the text layout.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/// The longest text a memo holds, its terminator included. Longer text is measured every time
#define MEASURE_MEMO_TEXT 8

/**
 * @brief One remembered measurement: the text, the font it was measured in, and how big it came
 * out.
 *
 * Each call site keeps its own, since the size also hangs on the box and the alignment it is
 * measured in, and a site always measures in the same ones.
 */
typedef struct
{
    GFont font;                    ///< The font it was measured in, or NULL while it holds nothing
    GSize size;                    ///< How big it came out
    char text[MEASURE_MEMO_TEXT];  ///< The text that was measured
} MeasureMemo;

/**
 * @brief How big @p text renders in @p font, measured again only when the text or the font is not
 * the one the memo holds.
 *
 * @param memo The call site's memo.
 * @param text The text to measure.
 * @param font The font to measure it in.
 * @param box The box to lay it out in, the same one on every call for this memo.
 * @param alignment How to align it in the box, the same on every call for this memo.
 * @return The rendered size in pixels.
 */
GSize measure_size(MeasureMemo *memo, const char *text, GFont font, GRect box, GTextAlignment alignment);

/**
 * @brief How wide @p text renders in @p font, the width of measure_size.
 *
 * @param memo The call site's memo.
 * @param text The text to measure.
 * @param font The font to measure it in.
 * @param box The box to lay it out in, the same one on every call for this memo.
 * @param alignment How to align it in the box, the same on every call for this memo.
 * @return The rendered width in pixels.
 */
int measure_width(MeasureMemo *memo, const char *text, GFont font, GRect box, GTextAlignment alignment);

/** @} */
