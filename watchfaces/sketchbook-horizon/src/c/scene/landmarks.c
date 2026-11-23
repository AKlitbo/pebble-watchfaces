/**
 * @file landmarks.c
 * @brief What the landmark scenes share: on a round screen, the two-post sign the temperature goes
 * on.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/landmarks.h"

#include "scene/reading.h"
#include "draw/letter.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

#if defined(PBL_ROUND)
// --- The two-post sign ---
#define SIGN_TOP 136       ///< Top of the board
#define SIGN_H 16          ///< How tall the board is
#define SIGN_FOOT 164      ///< Where the posts reach the ground

void landmarks_draw_sign(GContext *ctx, int x, int pad, int radius, GColor post, GColor board, GColor ink)
{
    char temp[16];
    int width;
    if (!reading_temp(temp, sizeof(temp), &width))
    {
        return;
    }

    GRect face = GRect(x - (width + pad * 2) / 2, SIGN_TOP, width + pad * 2, SIGN_H);

    graphics_context_set_stroke_color(ctx, post);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, GPoint(face.origin.x + 5, SIGN_TOP + SIGN_H), GPoint(face.origin.x + 5, SIGN_FOOT));
    graphics_draw_line(ctx, GPoint(face.origin.x + face.size.w - 6, SIGN_TOP + SIGN_H),
        GPoint(face.origin.x + face.size.w - 6, SIGN_FOOT));
    graphics_context_set_stroke_width(ctx, 1);

    graphics_context_set_fill_color(ctx, board);
    graphics_fill_rect(ctx, face, radius, GCornersAll);
    graphics_context_set_stroke_color(ctx, ink);
    graphics_draw_round_rect(ctx, face, radius);
    letter_board(ctx, temp, face, ink);
}
#endif

/** @} */
