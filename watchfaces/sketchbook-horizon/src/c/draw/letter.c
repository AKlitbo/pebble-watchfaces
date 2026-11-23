/**
 * @file letter.c
 * @brief Measures and letters short text on a board in Hand 16.
 *
 * @ingroup watchface-sketchbook
 */
#include "draw/letter.h"

#include "ui/fonts.h"
#include "draw/fonts.h"
#include "draw/measure.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

int letter_width(const char *text)
{
    // the one thing measured here is the temperature, which holds for half an hour at a time
    static MeasureMemo s_memo;

    return measure_width(&s_memo, text, fonts_get(FONT_HAND_16), GRect(0, 0, 120, 26), GTextAlignmentCenter);
}

void letter_board(GContext *ctx, const char *text, GRect board, GColor color)
{
    // the hand font hangs low in its line box, so the text starts above the board to land the
    // lettering in the middle of it
    graphics_context_set_text_color(ctx, color);
    graphics_draw_text(ctx, text, fonts_get(FONT_HAND_16),
        GRect(board.origin.x, board.origin.y - 2, board.size.w, board.size.h + 8),
        GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

/** @} */
