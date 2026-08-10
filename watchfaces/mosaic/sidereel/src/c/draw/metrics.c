/**
 * @file metrics.c
 * @brief This face's font facts table. Teko has about 3px of top padding and the Share
 * Tech Mono fonts have none. cap_h is approximate on the slots nobody has measured. top_pad
 * and line_h are the ones that really matter.
 *
 * Looked up by FontId, one row per slot layout.c load_fonts registers.
 *
 * @ingroup watchface-sidereel
 */
#include "mosaic/draw/metrics.h"
#include "draw/fonts.h"

// looked up by font slot. slots this face does not load fall back to a system font
// and get an all-zero record
static const FontMetric s_metrics[FONT_COUNT] = {
    // this face's own cut, read off the font file rather than guessed at. Teko-SemiBold is
    // 1000 units to the em with ascent 958, descent 475 and cap height 636, and its digits
    // measure exactly 0 to 636, so for a size in pixels:
    //   cap_h   = 0.636 * size   the digits' own height
    //   top_pad = 0.322 * size   ascent minus cap height, the clear run above them
    //   line_h  = 1.433 * size   ascent plus descent
    // at 56 that is 36 / 18 / 80
    [FONT_CLOCK_56] = { .top_pad = 18, .cap_h = 36, .line_h = 80 },

    // the big-digit night fonts. Teko renders its glyph well below the text box top (about
    // 0.36 of the point size) and the digit caps are about half the point size. measured in
    // the emulator so draw_digits can centre them properly
    [FONT_TEKO_96] = { .top_pad = 43, .cap_h = 46, .line_h = 96 },
    [FONT_TEKO_88] = { .top_pad = 40, .cap_h = 42, .line_h = 88 },
    [FONT_TEKO_72] = { .top_pad = 26, .cap_h = 36, .line_h = 72 },
    [FONT_TEKO_54] = { .top_pad = 3, .cap_h = 50, .line_h = 54 },
    [FONT_TEKO_46] = { .top_pad = 3, .cap_h = 42, .line_h = 46 },
    [FONT_TIME_SM] = { 0 },                                       // unloaded (system fallback)
    [FONT_STM_14]  = { .top_pad = 0, .cap_h = 10, .line_h = 14 },
    [FONT_DATE_SM] = { 0 },                                       // unloaded
    [FONT_DATE_XS] = { 0 },                                       // unloaded
    [FONT_TEKO_26] = { .top_pad = 3, .cap_h = 24, .line_h = 26 }, // left values use -3
    [FONT_TEKO_22] = { .top_pad = 0, .cap_h = 20, .line_h = 22 }, // shrink fallback draws at 0
    [FONT_TEKO_24] = { .top_pad = 2, .cap_h = 22, .line_h = 24 }, // verify in emulator
    [FONT_TEKO_34] = { .top_pad = 3, .cap_h = 30, .line_h = 34 }, // big values add design lift
    [FONT_COORD]   = { 0 },                                       // unloaded
    [FONT_STM_12]  = { .top_pad = 1, .cap_h = 9,  .line_h = 12 }, // header/date use -1
};

static const FontMetric s_zero = { 0 };

const FontMetric *metric_for(FontId id)
{
    if (id >= FONT_COUNT)
    {
        return &s_zero;
    }

    return &s_metrics[id];
}

GRect metric_baseline(FontId id, GRect anchored)
{
    anchored.origin.y -= metric_for(id)->top_pad;
    return anchored;
}
