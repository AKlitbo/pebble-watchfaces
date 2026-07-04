/**
 * @file widgets.c
 * @brief Painted overlays drawn over the baked VS Code frame by the engine's overlays
 * draw-slot: the battery percentage/icon, the weather condition glyph, and the bluetooth
 * glyph.
 */
#include "widgets.h"

#include "layout.h"
#include "ui/fonts.h"
#include "ui/icon_cache.h"
#include "theme/theme.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"
#include "ui/weather/icons.h"

/**
 * @brief Draw a battery icon outline with a fill that tracks the charge level.
 *
 * @param ctx The graphics context.
 * @param area The battery body rect (the nub is drawn just past its right edge).
 * @param level Battery charge level percentage (0..100).
 * @param color The outline + fill color when not critical.
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
    GRect nub = GRect(area.origin.x + area.size.w, area.origin.y + area.size.h / 4, 2, area.size.h / 2);
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

void widgets_draw_battery(GContext *ctx, int level)
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
    GColor color = (level <= 20) ? GColorRed : stats_color_for_theme(settings_u8(SETTING_THEME));

    if (display != BATTERY_DISPLAY_PERCENT)
    {
        draw_battery_icon(ctx, BATT_ICON, level, stats_color_for_theme(settings_u8(SETTING_THEME)));
    }

    if (display != BATTERY_DISPLAY_ICON)
    {
        char buffer[8];
        snprintf(buffer, sizeof(buffer), "%d%%", level);

        // with no icon the percentage slides right into the icon's vacated space
        GRect rect = (display == BATTERY_DISPLAY_PERCENT) ? BATT_RECT_WIDE : BATT_RECT;

        graphics_context_set_text_color(ctx, color);
        graphics_draw_text(ctx, buffer, fonts_get(FONT_VALUE), rect,
            GTextOverflowModeFill, GTextAlignmentRight, NULL);
    }
}

void widgets_draw_bt(GContext *ctx, bool connected)
{
    GBitmap *bmp = icon_get(connected ? RESOURCE_ID_ICON_BLUETOOTH : RESOURCE_ID_ICON_BLUETOOTH_SLASH);
    if (!bmp)
    {
        return;
    }

    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, bmp, BT_ICON);
}

void widgets_draw_weather(GContext *ctx, const char *condition)
{
    // the cache loads the glyph for a condition once and hands it back on later paints
    GBitmap *bmp = icon_get(wx_resource_for(condition));
    if (!bmp)
    {
        return;
    }

    // the white master is tinted to the theme's stats colour each paint so a theme swap
    // recolours it too
    icon_tint(bmp, stats_color_for_theme(settings_u8(SETTING_THEME)));

    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, bmp, WX_ICON);
}
