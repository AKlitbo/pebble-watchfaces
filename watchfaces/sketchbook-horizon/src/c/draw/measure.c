/**
 * @file measure.c
 * @brief Remembers how big a piece of text came out, so a repaint that measures the same text in
 * the same font skips the text layout.
 *
 * @ingroup watchface-sketchbook
 */
#include "draw/measure.h"

#include <string.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

GSize measure_size(MeasureMemo *memo, const char *text, GFont font, GRect box, GTextAlignment alignment)
{
    if (memo->font == font && strcmp(memo->text, text) == 0)
    {
        return memo->size;
    }

    GSize size = graphics_text_layout_get_content_size(text, font, box, GTextOverflowModeTrailingEllipsis,
        alignment);

    // text too long to hold is measured every time rather than cut down to fit, since a cut key
    // would match a different string that starts the same way
    size_t len = strlen(text);
    if (len < sizeof(memo->text))
    {
        memcpy(memo->text, text, len + 1);
        memo->font = font;
        memo->size = size;
    }
    else
    {
        memo->font = NULL;
    }

    return size;
}

int measure_width(MeasureMemo *memo, const char *text, GFont font, GRect box, GTextAlignment alignment)
{
    return measure_size(memo, text, font, box, alignment).w;
}

/** @} */
