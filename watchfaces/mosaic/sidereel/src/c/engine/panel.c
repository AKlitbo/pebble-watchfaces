/**
 * @file panel.c
 * @brief The panel chrome: the border, the header block, the checker dither and the label.
 *
 * The drawing matches gridlock's right down to the pixel offsets, because a 1x2 cell here is the
 * same 94x40 it is there and the module bodies are the same files. Nothing here resolves a grid,
 * though: no cells to place, no night layout, no per-module colour overrides. The colours come
 * straight off sidereel's palette.
 *
 * @ingroup watchface-sidereel
 */
#include "panel.h"

#include "draw/fonts.h"
#include "mosaic/draw/header_fonts.h"
#include "mosaic/draw/panel_styles.h"
#include "engine/grid_engine.h"
#include "mosaic/engine/vibrant_table.g.h"
#include "settings_schema.h"
#include "system/settings/settings.h"
#include "theme/custom_colors.h"
#include "ui/fonts.h"

#include <string.h>

/**
 * @addtogroup watchface-sidereel
 * @{
 */

GRect grid_anchor(GridCtx *gctx, GSize size, GAlign anchor, int dx, int dy)
{
    GRect r = GRect(0, 0, size.w, size.h);
    switch (anchor)
    {
        case GAlignTopLeft:      r.origin = GPoint(gctx->body.origin.x, gctx->body.origin.y); break;
        case GAlignTop:          r.origin = GPoint(gctx->body.origin.x + (gctx->body.size.w - size.w) / 2, gctx->body.origin.y); break;
        case GAlignTopRight:     r.origin = GPoint(gctx->body.origin.x + gctx->body.size.w - size.w, gctx->body.origin.y); break;
        case GAlignLeft:         r.origin = GPoint(gctx->body.origin.x, gctx->body.origin.y + (gctx->body.size.h - size.h) / 2); break;
        case GAlignCenter:       r.origin = GPoint(gctx->body.origin.x + (gctx->body.size.w - size.w) / 2, gctx->body.origin.y + (gctx->body.size.h - size.h) / 2); break;
        case GAlignRight:        r.origin = GPoint(gctx->body.origin.x + gctx->body.size.w - size.w, gctx->body.origin.y + (gctx->body.size.h - size.h) / 2); break;
        case GAlignBottomLeft:   r.origin = GPoint(gctx->body.origin.x, gctx->body.origin.y + gctx->body.size.h - size.h); break;
        case GAlignBottom:       r.origin = GPoint(gctx->body.origin.x + (gctx->body.size.w - size.w) / 2, gctx->body.origin.y + gctx->body.size.h - size.h); break;
        case GAlignBottomRight:  r.origin = GPoint(gctx->body.origin.x + gctx->body.size.w - size.w, gctx->body.origin.y + gctx->body.size.h - size.h); break;
    }
    r.origin.x += dx;
    r.origin.y += dy;
    return r;
}

// the header dither is a fixed 50% checker. plotting it pixel by pixel every repaint is slow, so
// it is baked once into a 1-bit palette bitmap and blitted, with the accent swapped in via the
// palette. the cell's top-left parity decides which of the two phase variants lands the checker on
// the right pixels. wide enough for the widest cell this face has
#define CHECKER_W 96
#define CHECKER_H 14

static GBitmap *s_checker[2];
static GColor   s_checker_pal[2][2];

static void build_checker(void)
{
    for (int phase = 0; phase < 2; phase++)
    {
        if (s_checker[phase])
        {
            continue;
        }

        s_checker_pal[phase][0] = GColorClear;
        s_checker_pal[phase][1] = GColorWhite; // recoloured to the accent on each draw

        GBitmap *bmp = gbitmap_create_blank_with_palette(GSize(CHECKER_W, CHECKER_H),
                           GBitmapFormat1BitPalette, s_checker_pal[phase], false);
        if (!bmp)
        {
            continue; // out of heap so the next draw has another go
        }

        uint8_t *data = gbitmap_get_data(bmp);
        uint16_t stride = gbitmap_get_bytes_per_row(bmp);
        for (int y = 0; y < CHECKER_H; y++)
        {
            for (int x = 0; x < CHECKER_W; x++)
            {
                if (((x + y) & 1) == phase)
                {
                    data[y * stride + (x >> 3)] |= (uint8_t)(0x80 >> (x & 7));
                }
            }
        }
        s_checker[phase] = bmp;
    }
}

/**
 * @brief Fills a rect with a 50% checkerboard pattern in `color`, blitting a cached bitmap.
 */
static void draw_checker(GContext *ctx, GRect area, GColor color)
{
    if (!s_checker[0] || !s_checker[1])
    {
        build_checker();
    }

    int phase = (area.origin.x + area.origin.y) & 1;
    GBitmap *bmp = s_checker[phase];
    if (!bmp)
    {
        return;
    }

    gbitmap_get_palette(bmp)[1] = color; // one write instead of repainting the pattern
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, bmp, area);
}

void panel_checker(GContext *ctx, GRect area, GColor color)
{
    draw_checker(ctx, area, color);
}

// the rendered width of a one-line header label in a given font
static int header_text_w(const char *text, GFont font)
{
    return graphics_text_layout_get_content_size(text, font, GRect(0, 0, 1000, HEADER_H),
                                                 GTextOverflowModeWordWrap, GTextAlignmentLeft).w;
}

/**
 * @brief Fits a header label to the block, truncating with a plain "..." when it is too long.
 *
 * GTextOverflowModeFill would append a native ellipsis glyph on overflow, but that glyph is
 * missing from most of the header fonts, so it lands on some and not others. Truncating here and
 * appending three periods gives a consistent "..." on every font.
 */
static const char *fit_header_label(const char *label, GFont font, int avail, char *buf, size_t n)
{
    if (header_text_w(label, font) <= avail)
    {
        return label;
    }

    int len = strlen(label);
    if (len > (int)n - 4)
    {
        len = (int)n - 4; // leave room for "..." and the NUL
    }

    for (; len > 0; len--)
    {
        memcpy(buf, label, len);
        strcpy(buf + len, "...");
        if (header_text_w(buf, font) <= avail)
        {
            return buf;
        }
    }

    strcpy(buf, "...");
    return buf;
}

/**
 * @brief Picks black or white for the header label so it stays readable on the accent block.
 *
 * The channels are weighted by how bright the eye sees each one.
 */
static GColor label_color_for(GColor accent)
{
    int lum = accent.r * 21 + accent.g * 72 + accent.b * 7; // 0 (black) to 300 (white)
    return lum >= 150 ? GColorBlack : GColorWhite;
}

void panel_draw(GContext *ctx, GRect bounds, ModuleType type, ModuleSize size)
{
    const ModuleDef *def = module_def(type);

    // a module that cannot sit in this cell draws nothing rather than drawing wrong. that is the
    // same gate gridlock's layout applies, kept so a stale id degrades to blank
    if (!def->body || !module_allows_size(type, size))
    {
        return;
    }

    uint8_t theme = settings_u8(SETTING_THEME);
    Palette base = palette_for_theme(theme);

    // a grouped panel borrows another's colours, so the two read as one family
    uint8_t colour_id = def->theme_alias ? def->theme_alias : (uint8_t)type;

    if (theme == THEME_VIBRANT)
    {
        ModuleColors vivid = MODULE_VIBRANT[colour_id];
        base.accent = gcolor_equal(vivid.accent, GColorClear) ? base.accent : vivid.accent;
        base.value = gcolor_equal(vivid.value, GColorClear) ? base.value : vivid.value;
        base.icon = gcolor_equal(vivid.icon, GColorClear) ? base.icon : vivid.icon;
        base.subtitle = gcolor_equal(vivid.subtitle, GColorClear) ? base.subtitle : vivid.subtitle;
    }
    else if (theme == THEME_CUSTOM)
    {
        SidereelCustomColor pick = sidereel_custom_color(colour_id);
        base.accent = pick.accent_set ? pick.accent : base.accent;
        base.value = pick.value_set ? pick.value : base.value;
        base.icon = pick.icon_set ? pick.icon : base.icon;
        // the caption follows the accent unless it was given its own colour
        base.subtitle = pick.subtitle_set ? pick.subtitle
                                          : (pick.accent_set ? pick.accent : base.subtitle);
    }

    const Palette *pal = &base;

    // a module can ask for the whole tile at a given size, and the user can ask for it too
    bool headerless = (def->headerless_sizes & (1 << size)) != 0
                      || sidereel_module_headerless((uint8_t)type, size);
    bool borderless = sidereel_module_borderless((uint8_t)type, size);

    GridCtx gctx = {
        .ctx = ctx,
        .bounds = bounds,
        .body = GRect(bounds.origin.x, bounds.origin.y + HEADER_H,
                      bounds.size.w, bounds.size.h - HEADER_H),
        .size = size,
        .color_value = pal->value,
        .color_subtitle = pal->subtitle,
        .color_accent = pal->accent,
        .color_icon = pal->icon,
        .color_label = label_color_for(pal->accent),
        .borderless = borderless,
        .headerless = headerless,
    };

    // the Panel Style pick is a corner radius, and 0 is the square classic frame. the header
    // block below rounds its own top corner to match, or it would square the border back off
    const uint8_t radius = panel_style_radius(sidereel_panel_style());

    if (!borderless)
    {
        graphics_context_set_stroke_color(ctx, gctx.color_accent);
        if (radius > 0)
        {
            graphics_draw_round_rect(ctx, gctx.bounds, radius);
        }
        else
        {
            graphics_draw_rect(ctx, gctx.bounds);
        }
    }

    // headerless drops the 14px strip and hands the module the whole tile. the body reads
    // gctx.headerless when its own layout depends on which it got
    if (headerless)
    {
        gctx.body = gctx.bounds;
        def->body(&gctx);
        return;
    }

    // --- header: solid label block plus a 50% checker across the rest of the strip ---
    // the spec's nudge seats the chosen font in the 14px strip
    const HeaderFontSpec *header = header_font_spec(settings_u8(SETTING_HEADER_FONT));
    const char *label_text = def->get_label ? def->get_label(gctx.size) : def->label;
    GFont header_font = fonts_get(FONT_HEADER);
    GSize text_size = graphics_text_layout_get_content_size(label_text, header_font,
                                                            GRect(0, 0, gctx.bounds.size.w, HEADER_H),
                                                            GTextOverflowModeFill, GTextAlignmentLeft);

    int label_w = text_size.w + PANEL_PAD * 2;
    if (label_w > gctx.bounds.size.w - 12)
    {
        label_w = gctx.bounds.size.w - 12;
    }

    char label_buf[24];
    label_text = fit_header_label(label_text, header_font, label_w - PANEL_PAD * 2,
                                  label_buf, sizeof(label_buf));

    int left = gctx.bounds.origin.x;
    int right = gctx.bounds.origin.x + gctx.bounds.size.w;
    int top = gctx.bounds.origin.y;
    int strip_h = HEADER_H - 1;   // last row is the divider

    draw_checker(ctx, GRect(left + label_w, top + 1, gctx.bounds.size.w - label_w, strip_h - 1),
                 gctx.color_accent);

    // the label block sits in the panel's top-left, so under a rounded style it has to carry
    // that corner itself rather than fill square over the border
    graphics_context_set_fill_color(ctx, gctx.color_accent);
    graphics_fill_rect(ctx, GRect(left, top, label_w, strip_h), radius,
                       radius > 0 ? GCornerTopLeft : GCornerNone);

    graphics_context_set_stroke_color(ctx, gctx.color_accent);
    graphics_draw_line(ctx, GPoint(left, top + HEADER_H - 1), GPoint(right - 1, top + HEADER_H - 1));

    GRect label_rect = GRect(left + PANEL_PAD + header->dx, top - 1 + header->dy,
                             label_w - PANEL_PAD * 2, HEADER_H);
    graphics_context_set_text_color(ctx, gctx.color_label);
    graphics_draw_text(ctx, label_text, header_font, label_rect,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    def->body(&gctx);
}

void panel_cleanup(void)
{
    for (int phase = 0; phase < 2; phase++)
    {
        if (s_checker[phase])
        {
            gbitmap_destroy(s_checker[phase]);
            s_checker[phase] = NULL;
        }
    }

    modules_cleanup();
}

/** @} */
