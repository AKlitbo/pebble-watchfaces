/**
 * @file widgets.c
 * @brief Painted overlays drawn over the baked frame by the engine's overlays draw-slot: the
 * battery, the weather condition glyph, and the Quiet Time and bluetooth status.
 *
 * @ingroup watchface-vscode
 */
#include "widgets.h"

#include "ui/fonts.h"
#include "draw/fonts.h"
#include "ui/icon_cache.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"
#include "ui/weather/icons.h"
#include "ui/readouts.h"

/**
 * @addtogroup watchface-vscode
 * @{
 */

// at or under this charge the battery turns red
#define BATT_LOW 20

// the battery cell's nub sticks out this far past its right edge
#define BATT_NUB_W 2

// the widest the battery percentage can be, 100% with room to spare
#define PERCENT_MAX_W 40

// the gauge is a row of this many blocks, each a tenth of the charge
#define GAUGE_BLOCKS 10

/**
 * @brief Draw a battery cell outline with a fill that tracks the charge level.
 *
 * @param ctx The graphics context.
 * @param area The battery body rect (the nub is drawn just past its right edge).
 * @param level Battery charge level percentage (0..100).
 * @param outline The outline and nub colour.
 * @param fill The fill colour.
 */
static void draw_battery_cell(GContext *ctx, GRect area, int level, GColor outline, GColor fill)
{
    graphics_context_set_stroke_color(ctx, outline);
    graphics_draw_rect(ctx, area);

    // nub on the right edge
    GRect nub = GRect(area.origin.x + area.size.w, area.origin.y + area.size.h / 4, BATT_NUB_W, area.size.h / 2);
    graphics_context_set_fill_color(ctx, outline);
    graphics_fill_rect(ctx, nub, 0, GCornerNone);

    // proportional fill inside the body (1px inset)
    int inner_w = area.size.w - 2;
    int fill_w = (level * inner_w + 50) / 100;
    if (fill_w > 0)
    {
        graphics_context_set_fill_color(ctx, fill);
        graphics_fill_rect(ctx, GRect(area.origin.x + 1, area.origin.y + 1, fill_w, area.size.h - 2), 0, GCornerNone);
    }
}

/**
 * @brief Draw a row of blocks, solid up to the charge level and shaded after it.
 *
 * @param ctx The graphics context.
 * @param area The whole row. Each block takes a tenth of it with a 1px gap after.
 * @param level Battery charge level percentage (0..100).
 * @param color The block colour.
 */
static void draw_battery_gauge(GContext *ctx, GRect area, int level, GColor color)
{
    int pitch = area.size.w / GAUGE_BLOCKS;
    int full = (level + 5) / 10;

    graphics_context_set_fill_color(ctx, color);
    graphics_context_set_stroke_color(ctx, color);

    for (int block = 0; block < GAUGE_BLOCKS; block++)
    {
        GRect cell = GRect(area.origin.x + block * pitch, area.origin.y, pitch - 1, area.size.h);

        if (block < full)
        {
            graphics_fill_rect(ctx, cell, 0, GCornerNone);
            continue;
        }

        // a checkerboard of pixels, the shaded block a DOS screen draws past the level
        for (int y = 0; y < cell.size.h; y++)
        {
            for (int x = (y & 1); x < cell.size.w; x += 2)
            {
                graphics_draw_pixel(ctx, GPoint(cell.origin.x + x, cell.origin.y + y));
            }
        }
    }
}

/**
 * @brief Draw a status word, such as LINK or OFF, in the desktop's readout font.
 *
 * @param ctx The graphics context.
 * @param layout The active desktop layout.
 * @param rect Where the word goes.
 * @param word The word.
 * @param color Its colour.
 */
static void draw_status_word(GContext *ctx, const DesktopLayout *layout, GRect rect, const char *word,
    GColor color)
{
    graphics_context_set_text_color(ctx, color);
    graphics_draw_text(ctx, word, fonts_get(layout->batt_font), rect,
        GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

/**
 * @brief Draw a white icon from the cache in a colour.
 *
 * @param ctx The graphics context.
 * @param resource The icon's resource id.
 * @param rect Where it goes.
 * @param color Its tint.
 */
static void draw_tinted(GContext *ctx, uint32_t resource, GRect rect, GColor color)
{
    GBitmap *bmp = icon_get(resource);
    if (!bmp)
    {
        return;
    }

    // tinted on every paint, so a theme or desktop change recolours it too
    icon_tint(bmp, color);

    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, bmp, rect);
}

/**
 * @brief Draw a status glyph with its visible art one gap left of the last item, and move the edge
 * past it.
 *
 * The glyphs leave different blank columns inside their boxes, so lining up the boxes would space
 * the art unevenly. Packing by the art keeps every visible gap the same.
 *
 * @param ctx The graphics context.
 * @param resource The icon's resource id.
 * @param row The glyph's row and size. Its x is worked out here.
 * @param color Its tint.
 * @param right The edge to pack against, moved left past this glyph and a gap.
 */
static void pack_glyph(GContext *ctx, uint32_t resource, GRect row, GColor color, int *right)
{
    IconMargins margins = icon_margins(resource);

    row.origin.x = *right - row.size.w + margins.e;
    draw_tinted(ctx, resource, row, color);
    *right = row.origin.x + margins.w - STATUS_GAP;
}

/**
 * @brief Draw the battery percentage in its fixed rect, or packed one gap left of the edge.
 *
 * @param ctx The graphics context.
 * @param layout The active desktop layout.
 * @param text The percentage.
 * @param color Its colour.
 * @param right The edge to pack against, moved left past the text and a gap, or NULL for the
 *   desktop's fixed rect.
 */
static void draw_percent(GContext *ctx, const DesktopLayout *layout, const char *text, GColor color, int *right)
{
    GFont font = fonts_get(layout->batt_font);
    graphics_context_set_text_color(ctx, color);

    if (!right)
    {
        graphics_draw_text(ctx, text, font, layout->batt_text, GTextOverflowModeFill, layout->batt_align, NULL);
        return;
    }

    // measured rather than given a fixed box, so what packs left of it sits one gap from the text
    // whatever its width
    GRect box = GRect(*right - PERCENT_MAX_W, layout->batt_text.origin.y, PERCENT_MAX_W, layout->batt_text.size.h);
    GSize size = graphics_text_layout_get_content_size(text, font, box, GTextOverflowModeFill, GTextAlignmentRight);

    graphics_draw_text(ctx, text, font, box, GTextOverflowModeFill, GTextAlignmentRight, NULL);
    *right -= size.w + STATUS_GAP;
}

void widgets_draw_status(GContext *ctx, const DesktopLayout *layout, int level, bool connected)
{
    if (level < 0)
    {
        level = 0;
    }

    if (level > 100)
    {
        level = 100;
    }

    bool low = level <= BATT_LOW;
    uint8_t display = settings_u8(SETTING_BATTERY_DISPLAY);
    bool show_icon = display != BATTERY_DISPLAY_PERCENT && layout->batt_icon.size.w;
    bool show_text = layout->batt_text_always || display != BATTERY_DISPLAY_ICON;
    bool show_bt = settings_u8(SETTING_BLUETOOTH_ICON);
    bool quiet = quiet_time_is_active();

    char percent[8];
    snprintf(percent, sizeof(percent), "%d%%", level);
    GColor text_color = low ? GColorRed : layout->batt_color;

    if (show_icon)
    {
        if (layout->batt_style == BATT_STYLE_GAUGE)
        {
            draw_battery_gauge(ctx, layout->batt_icon, level, low ? GColorRed : layout->batt_color);
        }
        else
        {
            draw_battery_cell(ctx, layout->batt_icon, level, layout->batt_color, low ? GColorRed : layout->batt_fill);
        }
    }

    // words on labelled lines, each in its own fixed spot
    if (layout->status_text)
    {
        if (show_text)
        {
            draw_percent(ctx, layout, percent, text_color, NULL);
        }

        if (show_bt)
        {
            draw_status_word(ctx, layout, layout->bt, connected ? "LINK" : "DOWN",
                connected ? layout->status_color : layout->status_off_color);
        }

        draw_status_word(ctx, layout, layout->quiet, quiet ? "ON" : "OFF",
            quiet ? layout->quiet_color : layout->status_off_color);
        return;
    }

    // glyphs packed leftwards from the battery icon, or from where its nub ends when it is hidden
    int right = show_icon ? layout->batt_icon.origin.x - STATUS_GAP
                          : layout->batt_icon.origin.x + layout->batt_icon.size.w + BATT_NUB_W;

    if (show_text)
    {
        draw_percent(ctx, layout, percent, text_color, layout->batt_text_always ? NULL : &right);
    }

    // the store owns the connect/disconnect vibe, so hiding the glyph only hides the glyph
    if (show_bt)
    {
        pack_glyph(ctx, connected ? RESOURCE_ID_ICON_BLUETOOTH : RESOURCE_ID_ICON_BLUETOOTH_SLASH, layout->bt,
            layout->status_color, &right);
    }

    if (quiet)
    {
        pack_glyph(ctx, RESOURCE_ID_ICON_QUIET_ON, layout->quiet, layout->status_color, &right);
    }
}

void widgets_draw_spaced_clock(GContext *ctx, const DesktopLayout *layout)
{
    if (!layout->clock_pitch)
    {
        return;
    }

    char text[12];
    readout_time(text, sizeof(text));

    const Zone *zone = &layout->zones[ZONE_TIME];
    GFont font = fonts_get(zone->font_id);

    int width = 0;
    for (const char *at = text; *at; at++)
    {
        width += (*at == ':') ? layout->clock_colon_pitch : layout->clock_pitch;
    }

    int x = zone->rect.origin.x + (zone->rect.size.w - width) / 2;
    graphics_context_set_text_color(ctx, zone->color);

    // a monospace font centres each glyph's ink in its advance, so centring the advance on the
    // glyph's own share centres the ink there too
    for (const char *at = text; *at; at++)
    {
        char glyph[2] = {*at, '\0'};
        int pitch = (*at == ':') ? layout->clock_colon_pitch : layout->clock_pitch;
        GSize size = graphics_text_layout_get_content_size(glyph, font, zone->rect, GTextOverflowModeFill,
            GTextAlignmentLeft);

        // 2px spare, since the layout wants the text inside the box rather than touching its edge
        GRect cell = GRect(x + (pitch - size.w) / 2 - 1, zone->rect.origin.y, size.w + 2, zone->rect.size.h);
        graphics_draw_text(ctx, glyph, font, cell, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
        x += pitch;
    }
}

void widgets_draw_weather(GContext *ctx, const DesktopLayout *layout, const char *condition)
{
    if (!layout->wx_icon.size.w)
    {
        return;
    }

    // the cache loads the glyph for a condition once and hands it back on later paints
    draw_tinted(ctx, wx_resource_for(condition), layout->wx_icon, layout->wx_color);
}

/** @} */
