/**
 * @file grid_engine.c
 * @brief The Gridlock grid, built on the shared slot engine. Each layout cell becomes a
 * draw-slot: the shared engine owns the per-cell clipped layer and the repaint, and this
 * file just paints one cell. A cell paints the usual bits (border, header label, hatch or
 * progress, value, subtitle, icon) or hands the module the whole frame when it sets a draw
 * override. The stores own the readings. When a reading changes the engine just repaints.
 * @ingroup gridlock_engine
 */
#include "engine/grid_engine.h"

#include "engine/layouts.h"
#include "engine/catalog.h"
#include "engine/vibrant_table.g.h"
#include "ui/fonts.h"
#include "draw/metrics.h"
#include "draw/header_fonts.h"
#include "theme/theme.h"
#include "system/settings/settings.h"
#include "settings_schema.h"

#define HEADER_H 14    // header strip height

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

// the cells of the current layout, held so the draw-slots can point at them
static ResolvedSlot s_resolved[GRIDLOCK_MAX_CELLS];

// the theme and its base palette, worked out once per build. every cell shares the same
// base, so this stays out of the per-cell path. a theme change goes through engine_rebuild
// (see on_settings_changed), which re-runs gridlock_build and refreshes these
static uint8_t s_theme;
static Palette s_base_pal;

// the header label's seating nudge for the user's chosen Header Font, worked out once per build
// alongside the theme. 0/0 for the default Share Tech Mono
static int8_t s_header_dx;
static int8_t s_header_dy;

// which side the solid label block sits on. the dither fills the other side
// a knob for later. not hooked up to settings yet
static bool s_header_label_right = false;

// the header dither is a fixed 50% checker. plotting it pixel by pixel every repaint is slow,
// so it is baked once into a 1-bit palette bitmap and blitted, with the accent swapped in via
// the palette (the pattern data never changes). the cell's top-left parity decides which of the
// two phase variants lands the checker on the right pixels. wide enough for a full-width panel
#define CHECKER_W 208
#define CHECKER_H 14

static GBitmap *s_checker[2];       // [0] paints where (x+y) is even, [1] where it is odd
static GColor   s_checker_pal[2][2]; // each bitmap's [transparent, accent] palette

// builds the two phase bitmaps. the "on" pixels carry palette index 1 (the accent, set per
// draw), the rest index 0 (GColorClear, left see-through under GCompOpSet). a phase that is
// already built is left alone so a call with one phase missing just fills the gap
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

void gridlock_engine_cleanup(void)
{
    // free the two cached checker bitmaps. built lazily on first header draw, so this is the
    // symmetric teardown (NULLing lets build_checker rebuild them if the face ever draws again)
    for (int phase = 0; phase < 2; phase++)
    {
        if (s_checker[phase])
        {
            gbitmap_destroy(s_checker[phase]);
            s_checker[phase] = NULL;
        }
    }
}

/**
 * @brief Fills a rect with a 50% checkerboard pattern in `color`, blitting a cached bitmap.
 *
 * @param ctx The graphics context.
 * @param area The rect to fill (in the layer's own coordinates).
 * @param color The pixel colour.
 */
static void draw_checker(GContext *ctx, GRect area, GColor color)
{
    // either phase can be missing on its own if the heap was tight so check both
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

// the rendered width of a one-line header label in a given font
static int header_text_w(const char *text, GFont font)
{
    return graphics_text_layout_get_content_size(text, font, GRect(0, 0, 1000, HEADER_H),
                                                 GTextOverflowModeWordWrap, GTextAlignmentLeft).w;
}

/**
 * @brief Fits a header label to the block, truncating with a plain "..." when it is too long.
 *
 * GTextOverflowModeFill would append a native ellipsis glyph on overflow, but that glyph is missing
 * from most of the header fonts (Electrolize has no U+2026 at all), so it showed on some fonts and
 * not others. Truncating here and appending three periods (every font has ".") gives a consistent
 * "..." on every font, and because the result fits, Fill never adds its own ellipsis on top.
 *
 * @param label The full label.
 * @param font The header font.
 * @param avail The block's content width in pixels.
 * @param buf A scratch buffer for the truncated string.
 * @param n The size of buf.
 * @return label when it fits, otherwise the longest prefix of it that fits with a trailing "...".
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
 * @brief Draws the usual panel frame and body for a module in the layer's own space.
 *
 * @param gctx The grid context.
 * @param def The module description.
 */
static void draw_panel(GridCtx *gctx, const ModuleDef *def, bool headerless, bool borderless)
{
    // remember the frame state so helpers can adapt (EDGE_PAD, headerless font scaling)
    gctx->borderless = borderless;
    gctx->headerless = headerless;

    // the outer outline is identical in the headed and headerless layouts, so draw it once here.
    // borderless skips it for a frameless look, independent of the header
    if (!borderless)
    {
        graphics_context_set_stroke_color(gctx->ctx, gctx->color_accent);
        graphics_draw_rect(gctx->ctx, gctx->bounds);
    }

    // headerless drops the 14px header strip and hands the module the whole tile. the body
    // reads gctx->headerless when its layout depends on which it got (e.g. the analog clock)
    if (headerless)
    {
        gctx->body = gctx->bounds;
        if (def->body)
        {
            def->body(gctx);
        }
        return;
    }

    // --- header. solid label block plus a 50% checker pattern across the strip ---
    // the header font is the user's Header Font pick (registered as FONT_HEADER). s_header_dx
    // and s_header_dy seat that font in the strip. worked out once per build in gridlock_build
    const char *label_text = def->get_label ? def->get_label(gctx->size) : def->label;
    GFont header_font = fonts_get(FONT_HEADER);
    GSize text_size = graphics_text_layout_get_content_size(label_text, header_font,
                                                            GRect(0, 0, gctx->bounds.size.w, HEADER_H),
                                                            GTextOverflowModeFill, GTextAlignmentLeft);

    int label_w = text_size.w + PANEL_PAD * 2;
    if (label_w > gctx->bounds.size.w - 12)
    {
        label_w = gctx->bounds.size.w - 12;
    }

    // fit the label to the block, adding a plain "..." when it overflows so truncation looks the
    // same on every font (see fit_header_label). the block width itself stays as sized above
    char label_buf[24];
    label_text = fit_header_label(label_text, header_font, label_w - PANEL_PAD * 2,
                                  label_buf, sizeof(label_buf));

    int left = gctx->bounds.origin.x;
    int right = gctx->bounds.origin.x + gctx->bounds.size.w;
    int top = gctx->bounds.origin.y;
    int strip_h = HEADER_H - 1;   // last row is the divider

    int block_x = s_header_label_right ? right - label_w : left;
    int dither_x = s_header_label_right ? left : block_x + label_w;
    int dither_w = gctx->bounds.size.w - label_w;

    // 1. checker dither fills the side opposite the label
    draw_checker(gctx->ctx, GRect(dither_x, top + 1, dither_w, strip_h - 1), gctx->color_accent);

    // 2. solid block behind the label
    graphics_context_set_fill_color(gctx->ctx, gctx->color_accent);
    graphics_fill_rect(gctx->ctx, GRect(block_x, top, label_w, strip_h), 0, GCornerNone);

    // 3. bottom border
    graphics_context_set_stroke_color(gctx->ctx, gctx->color_accent);
    graphics_draw_line(gctx->ctx, GPoint(left, top + HEADER_H - 1),
                            GPoint(right - 1, top + HEADER_H - 1));

    // 4. label text. flipped colour, left-aligned a PANEL_PAD in from the block so a label too
    // long for the tile clips cleanly on the right instead of shifting to fit. s_header_dx/dy seat
    // the chosen font (0/0 for the default Share Tech Mono, so this is the usual PANEL_PAD / top - 1)
    GRect label_rect = GRect(block_x + PANEL_PAD + s_header_dx, top - 1 + s_header_dy,
                             label_w - PANEL_PAD * 2, HEADER_H);
    graphics_context_set_text_color(gctx->ctx, gctx->color_label);
    graphics_draw_text(gctx->ctx, label_text, header_font, label_rect,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    // a module paints its own body (e.g. the battery gauge) under the shared header above
    if (def->body)
    {
        def->body(gctx);
    }
}

/**
 * @brief Picks black or white for the header label so it stays readable on the accent block.
 *
 * The label sits on top of the solid accent colour, so a light accent wants black text and a
 * dark accent wants white. The channels are weighted by how bright the eye sees each one.
 *
 * @param accent The accent colour the label sits on.
 * @return GColorBlack on a light accent and GColorWhite on a dark one.
 */
static GColor label_color_for(GColor accent)
{
    int lum = accent.r * 21 + accent.g * 72 + accent.b * 7; // 0 (black) to 300 (white)
    return lum >= 150 ? GColorBlack : GColorWhite;
}

/**
 * @brief Draw-slot callback: paints one cell's module. `data` points at the cell's
 * ResolvedSlot and `bounds` is the cell's own (layer-local) rect.
 *
 * @param ctx The graphics context.
 * @param bounds The cell bounds in its own coordinates.
 * @param data The ResolvedSlot for this cell.
 */
static void cell_draw(GContext *ctx, GRect bounds, const void *data)
{
    const ResolvedSlot *slot = data;
    const ModuleDef *def = module_def(slot->module);

    // grouped panels borrow their primary's colours via theme_alias (the daily forecast reads
    // as the hourly panel, the watchlist as the stock panel). the header/border flags stay per
    // size and per module id, so the two panels still toggle independently
    uint8_t theme_module = def->theme_alias ? def->theme_alias : slot->module;

    uint8_t theme = s_theme;
    Palette pal = s_base_pal;

    GColor c_accent = pal.accent;
    GColor c_value = pal.value;
    GColor c_subtitle = pal.subtitle;
    GColor c_icon = pal.icon;

    // VIBRANT paints each module's own colours on top of the mono base (GColorClear means
    // keep the base). MONO MONO_INVERSE and CUSTOM stay on the plain base palette.
    if (theme == THEME_VIBRANT)
    {
        const ModuleColors *v = &MODULE_VIBRANT[theme_module];
        if (!gcolor_equal(v->accent, GColorClear))   c_accent = v->accent;
        if (!gcolor_equal(v->value, GColorClear))    c_value = v->value;
        if (!gcolor_equal(v->icon, GColorClear))     c_icon = v->icon;
        if (!gcolor_equal(v->subtitle, GColorClear)) c_subtitle = v->subtitle;
    }

    // CUSTOM starts from the plain mono look. the config page sets only the channels it
    // wants and anything left alone stays mono.
    if (theme == THEME_CUSTOM)
    {
        GridlockCustomColor custom = gridlock_custom_color(theme_module);
        if (custom.accent_set)
        {
            c_accent = custom.accent;
            c_subtitle = custom.accent; // the caption follows the accent by default
        }
        if (custom.value_set)    c_value = custom.value;
        if (custom.icon_set)     c_icon = custom.icon;
        if (custom.subtitle_set) c_subtitle = custom.subtitle; // its own colour wins when set
    }

    GridCtx gctx = {
        .ctx = ctx,
        .bounds = bounds,
        .body = GRect(bounds.origin.x, bounds.origin.y + HEADER_H, bounds.size.w, bounds.size.h - HEADER_H),
        .size = slot->msize,
        .color_value = c_value,
        .color_subtitle = c_subtitle,
        .color_accent = c_accent,
        .color_icon = c_icon,
        .color_label = label_color_for(c_accent)
    };

    // every module goes through draw_panel. the header is dropped for always-headerless
    // modules (the clock bar) or when this size's toggle hides it. the flags are per size AND
    // per module (its own id, not the theme head) so a grouped panel keeps its own
    // header/border toggle even while it shares its primary's colour
    bool headerless = (def->headerless_sizes & (1 << slot->msize))
                   || gridlock_module_headerless(slot->module, slot->msize);
    bool borderless = gridlock_module_borderless(slot->module, slot->msize);
    draw_panel(&gctx, def, headerless, borderless);
}

uint8_t gridlock_build(EngineSlot *out, uint8_t max, GRect bounds)
{
    (void)bounds;  // the layout resolves cells to absolute rects itself

    // the theme and base palette are the same for every cell, so resolve them once here
    // rather than per cell in cell_draw. this runs again on a theme change via engine_rebuild
    s_theme = settings_u8(SETTING_THEME);
    s_base_pal = palette_for_theme(s_theme);

    // the header font's seating nudge is the same for every cell too, so resolve it here from
    // the user's Header Font pick (the font handle itself lives under FONT_HEADER, set in main)
    const HeaderFontSpec *header = header_font_spec(settings_u8(SETTING_HEADER_FONT));
    s_header_dx = header->dx;
    s_header_dy = header->dy;

    uint8_t count = layouts_build(s_resolved, GRIDLOCK_MAX_CELLS);
    if (count > max)
    {
        count = max;
    }

    for (uint8_t i = 0; i < count; i++)
    {
        // a draw-slot per cell: the shared engine makes the clipped layer, we paint it. tags
        // are the module's feature mask so a store change only repaints the cells that read it
        out[i] = (EngineSlot){
            .frame = s_resolved[i].frame,
            .draw = cell_draw,
            .data = &s_resolved[i],
            .tags = module_def(s_resolved[i].module)->features,
        };
    }

    return count;
}
