/**
 * @file widgets.c
 * @brief Painted overlays drawn over the baked VS Code frame by the engine's overlays
 * draw-slot: the weather condition glyph, and the battery, bluetooth, and Quiet Time status in
 * the tab strip.
 *
 * @ingroup watchface-vscode
 */
#include "widgets.h"

#include "layout.h"
#include "ui/fonts.h"
#include "draw/fonts.h"
#include "ui/icon_cache.h"
#include "theme/theme.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"
#include "ui/weather/icons.h"

/**
 * @addtogroup watchface-vscode
 * @{
 */

// the battery cell's nub sticks out this far past its right edge
#define BATT_NUB_W 2

// the widest the battery percentage can be, 100% in Share Tech Mono 14 with room to spare
#define STATUS_TEXT_MAX_W 40

/**
 * @brief Draw a battery icon outline with a fill that tracks the charge level.
 *
 * @param ctx The graphics context.
 * @param area The battery body rect (the nub is drawn just past its right edge).
 * @param level Battery charge level percentage (0..100).
 * @param color The outline + fill colour when not critical.
 */
static void draw_battery_icon(GContext *ctx, GRect area, int level, GColor color)
{
    if (level < 0)
    {
        level = 0;
    }

    if (level > 100)
    {
        level = 100;
    }

    // outlined body
    graphics_context_set_stroke_color(ctx, color);
    graphics_draw_rect(ctx, area);

    // nub on the right edge
    GRect nub = GRect(area.origin.x + area.size.w, area.origin.y + area.size.h / 4, BATT_NUB_W, area.size.h / 2);
    graphics_context_set_fill_color(ctx, color);
    graphics_fill_rect(ctx, nub, 0, GCornerNone);

    // proportional fill inside the body (1px inset) red when critical
    int inner_w = area.size.w - 2;
    int fill_w = (level * inner_w + 50) / 100;
    if (fill_w > 0)
    {
        GColor fill = (level <= 20) ? GColorRed : color;
        graphics_context_set_fill_color(ctx, fill);
        graphics_fill_rect(ctx, GRect(area.origin.x + 1, area.origin.y + 1, fill_w, area.size.h - 2), 0, GCornerNone);
    }
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

    // tinted on every paint, so a theme change recolours it too
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

void widgets_draw_status(GContext *ctx, int level, bool connected)
{
    if (level < 0)
    {
        level = 0;
    }

    if (level > 100)
    {
        level = 100;
    }

    uint8_t display = settings_u8(SETTING_BATTERY_DISPLAY);

    // the glyphs take the stats colour rather than staying white, since the tab strip is white on
    // the Light theme
    GColor color = stats_color_for_theme(settings_u8(SETTING_THEME));

    // the right edge the next item packs against, moving left as each one is drawn
    int right = BATT_ICON.origin.x + BATT_ICON.size.w + BATT_NUB_W;

    if (display != BATTERY_DISPLAY_PERCENT)
    {
        draw_battery_icon(ctx, BATT_ICON, level, color);
        right = BATT_ICON.origin.x - STATUS_GAP;
    }

    if (display != BATTERY_DISPLAY_ICON)
    {
        char buffer[8];
        snprintf(buffer, sizeof(buffer), "%d%%", level);

        // measured rather than given a fixed box, so the glyphs left of it sit one gap from the
        // text whatever its width
        GFont font = fonts_get(FONT_STM_14);
        GRect box = GRect(right - STATUS_TEXT_MAX_W, BATT_TEXT.origin.y, STATUS_TEXT_MAX_W, BATT_TEXT.size.h);
        GSize size = graphics_text_layout_get_content_size(buffer, font, box, GTextOverflowModeFill,
            GTextAlignmentRight);

        graphics_context_set_text_color(ctx, (level <= 20) ? GColorRed : color);
        graphics_draw_text(ctx, buffer, font, box, GTextOverflowModeFill, GTextAlignmentRight, NULL);
        right -= size.w + STATUS_GAP;
    }

    // honour the show/hide setting. the store owns the connect/disconnect vibe
    if (settings_u8(SETTING_BLUETOOTH_ICON))
    {
        pack_glyph(ctx, connected ? RESOURCE_ID_ICON_BLUETOOTH : RESOURCE_ID_ICON_BLUETOOTH_SLASH, BT_ICON, color,
            &right);
    }

    if (quiet_time_is_active())
    {
        pack_glyph(ctx, RESOURCE_ID_ICON_QUIET_ON, QUIET_ICON, color, &right);
    }
}

void widgets_draw_weather(GContext *ctx, const char *condition)
{
    // the cache loads the glyph for a condition once and hands it back on later paints
    draw_tinted(ctx, wx_resource_for(condition), WX_ICON, stats_color_for_theme(settings_u8(SETTING_THEME)));
}

/** @} */
