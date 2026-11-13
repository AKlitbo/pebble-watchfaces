/**
 * @file music_now_playing.c
 * @brief The Now Playing panel, a full width bar with no header, at two heights. The album art fills
 * the tile's height on the left, and the title sits over the artist beside it. The 2x4 adds a row
 * underneath saying whether the track plays or is paused.
 *
 * Everything comes from the Pebble app's music plugin. With no art yet, nothing playing, or plugins
 * off, the art's square shows a note, and with nothing to play the text says which.
 *
 * @ingroup gridlock_mod_music
 */
#include "engine/grid_engine.h"
#include "music_now_playing.h"

#include "io/stores/music_store.h"
#include "ui/fonts.h"

#define ART_INSET 2 ///< the gap between the border and the cover, so the border never touches it
#define TEXT_GAP 6  ///< between the art and the text

// status lines are Teko 22
#define SMALL_FONT FONT_TEKO_22
#define SMALL_BOX_H 30 ///< one line of Teko 22 with room for its descenders, short of a second line
#define SMALL_DROP 8   ///< Teko 22 leaves this much empty above its capitals
#define SMALL_CAP_H 14 ///< how tall Teko 22's capitals draw
#define STATUS_GAP 6   ///< between the two words of a status stacked on the 2x4

// the 2x4's title is a size up, and falls back to Teko 18 when it is too wide for the room
#define BIG_FONT FONT_TEKO_24
#define BIG_BOX_H 33
#define BIG_DROP 9

// a track's title and artist are Teko 18 everywhere else, so a long name fits a useful amount before
// it trails off
#define TINY_FONT FONT_TEKO_18
#define TINY_BOX_H 25
#define TINY_DROP 7  ///< Teko 18 leaves this much empty above its capitals
#define TINY_CAP_H 11

// the 1x4's title and artist, by where their capitals start. the title's tall letters and the artist's
// foot each sit 5px in from the border, like the 2x4
#define SMALL_TITLE_TOP 7
#define SMALL_ARTIST_TOP 23

// the 2x4 stacks the title, the artist, and the play row, each by where its capitals start. the two
// Teko lines sit together up top and the play row at the foot, each 5px in from the border
#define BIG_TITLE_TOP 6
#define BIG_ARTIST_TOP 30
#define BIG_STATE_TOP 69

#define STATE_CAP_H 8 ///< Share Tech Mono 12's capitals, and the height of the mark and the bars
#define STATE_DROP 4  ///< Share Tech Mono 12 leaves this much empty above its capitals

// the little equalizer at the end of the play row. the bars stand while the track plays and lie flat
// while it is paused
#define BAR_W 3
#define BAR_GAP 2
#define BAR_COUNT 5
static const uint8_t BAR_HEIGHTS[BAR_COUNT] = {4, 8, 5, 7, 3};

/**
 * @brief Draws one line of text with its capitals starting at a given height.
 *
 * @param gctx The grid context.
 * @param text The words. A line too long for the room ends in an ellipsis.
 * @param font The font, Teko 18, 22, or 24.
 * @param color The text colour.
 * @param x Where the line starts.
 * @param cap_y Where its capitals start.
 */
static void draw_line(GridCtx *gctx, const char *text, FontId font, GColor color, int x, int cap_y)
{
    int right = gctx->body.origin.x + gctx->body.size.w - EDGE_PAD(gctx);
    int drop = font == BIG_FONT ? BIG_DROP : font == TINY_FONT ? TINY_DROP : SMALL_DROP;
    int box_h = font == BIG_FONT ? BIG_BOX_H : font == TINY_FONT ? TINY_BOX_H : SMALL_BOX_H;

    graphics_context_set_text_color(gctx->ctx, color);
    graphics_draw_text(gctx->ctx, text, fonts_get(font), GRect(x, cap_y - drop, right - x, box_h),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

/**
 * @brief Draws the empty cover: an outlined square with a music note in it, for no art.
 *
 * @param gctx The grid context.
 * @param slot The art's square.
 */
static void draw_empty_cover(GridCtx *gctx, GRect slot)
{
    graphics_context_set_stroke_color(gctx->ctx, gctx->color_accent);
    graphics_draw_rect(gctx->ctx, slot);

    // a quaver: a round head, a stem up its right side, and a flag off the top
    GPoint centre = grect_center_point(&slot);
    GPoint head = GPoint(centre.x - 3, centre.y + 6);
    int stem_x = head.x + 3;
    int stem_top = centre.y - 8;

    graphics_context_set_fill_color(gctx->ctx, gctx->color_icon);
    graphics_fill_circle(gctx->ctx, head, 3);
    graphics_context_set_stroke_color(gctx->ctx, gctx->color_icon);
    graphics_draw_line(gctx->ctx, GPoint(stem_x, head.y), GPoint(stem_x, stem_top));
    graphics_draw_line(gctx->ctx, GPoint(stem_x, stem_top), GPoint(stem_x + 5, stem_top + 4));
}

/**
 * @brief Draws a cover bigger than its square shrunk to fit, one pixel at a time.
 *
 * The phone sends one size of art, the 2x4's, and the SDK has no call that scales a bitmap. So the
 * 1x4 picks the nearest pixel of the cover for each pixel of its square. That costs about thirteen
 * hundred pixel draws a repaint and no second bitmap, so there is nothing to keep in step with the
 * cover when it changes.
 *
 * @param gctx The grid context.
 * @param art The cover, a 4-bit palettized bitmap.
 * @param slot The square to fill.
 */
static void draw_shrunk(GridCtx *gctx, const GBitmap *art, GRect slot)
{
    GSize size = gbitmap_get_bounds(art).size;
    const uint8_t *pixels = gbitmap_get_data(art);
    const GColor *palette = gbitmap_get_palette(art);
    uint16_t row_bytes = gbitmap_get_bytes_per_row(art);

    for (int y = 0; y < slot.size.h; y++)
    {
        const uint8_t *row = pixels + (y * size.h / slot.size.h) * row_bytes;

        for (int x = 0; x < slot.size.w; x++)
        {
            // two pixels to a byte, the first in the high half
            int from_x = x * size.w / slot.size.w;
            uint8_t pair = row[from_x / 2];
            GColor color = palette[(from_x % 2) ? (pair & 0x0F) : (pair >> 4)];

            // colours the phone left out of the palette are see-through, so the tile shows through them
            if (color.a == 0)
            {
                continue;
            }
            graphics_context_set_stroke_color(gctx->ctx, color);
            graphics_draw_pixel(gctx->ctx, GPoint(slot.origin.x + x, slot.origin.y + y));
        }
    }
}

/**
 * @brief Draws the album art in its square, or the empty cover with none.
 *
 * @param gctx The grid context.
 * @param slot The art's square.
 */
static void draw_art(GridCtx *gctx, GRect slot)
{
    const GBitmap *art = music_store_artwork();

    if (!art)
    {
        draw_empty_cover(gctx, slot);
        return;
    }

    GSize size = gbitmap_get_bounds(art).size;
    if (size.w > slot.size.w || size.h > slot.size.h)
    {
        draw_shrunk(gctx, art, slot);
        return;
    }

    // the plugin can send art smaller than it was asked for, so it is centred in the square
    GRect frame = GRect(slot.origin.x + (slot.size.w - size.w) / 2, slot.origin.y + (slot.size.h - size.h) / 2,
                        size.w, size.h);

    // colours the phone left out of the palette are see-through, so the tile shows through them
    graphics_context_set_compositing_mode(gctx->ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(gctx->ctx, art, frame);
}

/**
 * @brief Draws the 2x4's bottom row: a play or pause mark, the word for it, and the equalizer.
 *
 * @param gctx The grid context.
 * @param x Where the row starts.
 * @param cap_y Where its capitals start.
 */
static void draw_state_row(GridCtx *gctx, int x, int cap_y)
{
    bool playing = music_store_playing();
    int bottom = cap_y + STATE_CAP_H;

    graphics_context_set_stroke_color(gctx->ctx, gctx->color_icon);
    graphics_context_set_fill_color(gctx->ctx, gctx->color_icon);
    if (playing)
    {
        // a triangle pointing right, built from columns that lose a pixel top and bottom toward its tip
        for (int column = 0; column < STATE_CAP_H / 2; column++)
        {
            graphics_draw_line(gctx->ctx, GPoint(x + column, cap_y + column), GPoint(x + column, bottom - 1 - column));
        }
    }
    else
    {
        graphics_fill_rect(gctx->ctx, GRect(x, cap_y, 2, STATE_CAP_H), 0, GCornerNone);
        graphics_fill_rect(gctx->ctx, GRect(x + 4, cap_y, 2, STATE_CAP_H), 0, GCornerNone);
    }

    int right = gctx->body.origin.x + gctx->body.size.w - EDGE_PAD(gctx);
    int bars_w = BAR_COUNT * BAR_W + (BAR_COUNT - 1) * BAR_GAP;
    int text_x = x + 10;

    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, playing ? "PLAYING" : "PAUSED", fonts_get(FONT_STM_12),
                       GRect(text_x, cap_y - STATE_DROP, right - bars_w - 4 - text_x, 14), GTextOverflowModeFill,
                       GTextAlignmentLeft, NULL);

    graphics_context_set_fill_color(gctx->ctx, gctx->color_accent);
    for (int bar = 0; bar < BAR_COUNT; bar++)
    {
        int height = playing ? BAR_HEIGHTS[bar] : 2;
        int bar_x = right - bars_w + bar * (BAR_W + BAR_GAP);
        graphics_fill_rect(gctx->ctx, GRect(bar_x, bottom - height, BAR_W, height), 0, GCornerNone);
    }
}

/**
 * @brief Picks the font for the 2x4's title, Teko 24 when it fits on one line and Teko 18 when not.
 *
 * @param title The title.
 * @param width The room beside the art.
 * @return The font to draw it in.
 */
static FontId big_title_font(const char *title, int width)
{
    GSize size = graphics_text_layout_get_content_size(title, fonts_get(BIG_FONT), GRect(0, 0, 1000, BIG_BOX_H),
                                                       GTextOverflowModeFill, GTextAlignmentLeft);
    return size.w <= width ? BIG_FONT : TINY_FONT;
}

static void music_now_playing_body(GridCtx *gctx)
{
    GRect body = gctx->body;
    bool big = gctx->size == MSIZE_2x4;
    int side = body.size.h - ART_INSET * 2;
    GRect slot = GRect(body.origin.x + ART_INSET, body.origin.y + ART_INSET, side, side);
    int text_x = slot.origin.x + side + TEXT_GAP;
    int middle = body.origin.y + body.size.h / 2;

    const char *title = music_store_title();
    const char *artist = music_store_artist();

    // the store reads as nothing playing with plugins off too, so the plugin is asked about first. each
    // status is one line on the 1x4 and its two words stacked on the 2x4
    const char *status = NULL;
    const char *upper = NULL;
    const char *lower = NULL;
    if (music_store_no_plugin())
    {
        status = "PLUGINS OFF";
        upper = "PLUGINS";
        lower = "OFF";
    }
    else if (!music_store_has_track())
    {
        status = "NOTHING PLAYING";
        upper = "NOTHING";
        lower = "PLAYING";
    }

    // a status always sits beside the empty cover, since the art clears in a message of its own and
    // the last song's cover can still be held for a moment after the track has gone
    if (status)
    {
        draw_empty_cover(gctx, slot);
        if (big)
        {
            // the two words sit as a pair around the middle, a line's gap apart
            int upper_top = middle - SMALL_CAP_H - STATUS_GAP / 2;
            draw_line(gctx, upper, SMALL_FONT, gctx->color_subtitle, text_x, upper_top);
            draw_line(gctx, lower, SMALL_FONT, gctx->color_subtitle, text_x, upper_top + SMALL_CAP_H + STATUS_GAP);
            return;
        }
        draw_line(gctx, status, SMALL_FONT, gctx->color_subtitle, text_x, middle - SMALL_CAP_H / 2 + 1);
        return;
    }

    draw_art(gctx, slot);

    // a track with only one of its lines shows that one in the middle
    if (!title[0] || !artist[0])
    {
        draw_line(gctx, title[0] ? title : artist, TINY_FONT, gctx->color_value, text_x, middle - TINY_CAP_H / 2);
        return;
    }

    if (big)
    {
        int width = body.origin.x + body.size.w - EDGE_PAD(gctx) - text_x;
        FontId font = big_title_font(title, width);

        draw_line(gctx, title, font, gctx->color_value, text_x, body.origin.y + BIG_TITLE_TOP);
        draw_line(gctx, artist, TINY_FONT, gctx->color_subtitle, text_x, body.origin.y + BIG_ARTIST_TOP);
        draw_state_row(gctx, text_x, body.origin.y + BIG_STATE_TOP);
        return;
    }

    // the title over the artist, with a gap between for the title's descenders
    draw_line(gctx, title, TINY_FONT, gctx->color_value, text_x, body.origin.y + SMALL_TITLE_TOP);
    draw_line(gctx, artist, TINY_FONT, gctx->color_subtitle, text_x, body.origin.y + SMALL_ARTIST_TOP);
}

const ModuleDef mod_music_now_playing_def = {
    .label = "NOW PLAYING",
    .sizes = SZ_1x4 | SZ_2x4,
    .features = FEATURE_MUSIC,
    .body = music_now_playing_body,
    // the art takes the tile's whole height, so the header never shows
    .headerless_sizes = SZ_1x4 | SZ_2x4,
};
