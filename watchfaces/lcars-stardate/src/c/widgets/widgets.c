/**
 * @file widgets.c
 * @brief Painted overlays: battery gauge, bar labels, weather/heart/feet/bluetooth glyphs,
 * drawn over the baked frame by the engine's overlays draw-slot.
 *
 * @ingroup watchface-lcars
 */
#include "widgets.h"

#include "layout.h"
#include "system/settings/settings.h"
#include "theme/theme.h"
#include "ui/icon_cache.h"
#include "ui/weather/icons.h"

static GFont s_font_label;  // Antonio 10 bar labels

// --- LCARS skin primitives ---
// stateless painters (ctx in and pixels out) for the chrome only LCARS faces draw:
// the holder-box label and the segmented battery gauge

/**
 * @addtogroup watchface-lcars
 * @{
 */

/**
 * @brief Draw a left-aligned label inside a black LCARS holder box.
 *
 * @param ctx The graphics context.
 * @param area The area to draw in.
 * @param text The label text.
 * @param font The font to use.
 * @param text_color The text colour.
 * @param pad Padding inside the box.
 */
static void lcars_label(GContext *ctx, GRect area, const char *text, GFont font, GColor text_color, int pad)
{
    GSize sz = graphics_text_layout_get_content_size(text, font, area, GTextOverflowModeFill, GTextAlignmentLeft);

    int box_w = sz.w + pad * 2;
    if (box_w > area.size.w)
    {
        box_w = area.size.w;
    }

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(area.origin.x, area.origin.y, box_w, area.size.h), 0, GCornerNone);

    graphics_context_set_text_color(ctx, text_color);
    graphics_draw_text(ctx, text, font,
        GRect(area.origin.x + pad, area.origin.y - 1, box_w - pad, area.size.h + 4),
        GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

/**
 * @brief Draw the 5-segment battery gauge, colour-coded by level.
 *
 * @param ctx The graphics context.
 * @param area The bounding box of the battery icon.
 * @param level Battery charge level percentage.
 * @param accent_color The outline and nub colour.
 * @param fill_color The lit-segment colour (already level- and theme-resolved).
 */
static void lcars_battery_gauge(GContext *ctx, GRect area, int level, GColor accent_color, GColor fill_color)
{
    if (level < 0)
    {
        level = 0;
    }

    if (level > 100)
    {
        level = 100;
    }

    GRect body = area;
    GRect nub = GRect(area.origin.x + area.size.w, area.origin.y + area.size.h / 4, 3, area.size.h / 2);

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, body, 2, GCornersAll);
    graphics_fill_rect(ctx, nub, 1, GCornersRight);

    GRect inner = GRect(body.origin.x + 2, body.origin.y + 2, body.size.w - 4, body.size.h - 4);

    const int segments = 5, gap = 2;
    int seg_w = (inner.size.w - (segments - 1) * gap) / segments;
    if (seg_w < 1)
    {
        seg_w = 1;
    }

    int lit = (level * segments + 50) / 100;
    if (lit == 0 && level > 0)
    {
        lit = 1;
    }

    for (int i = 0; i < segments; i++)
    {
        GRect cell = GRect(inner.origin.x + i * (seg_w + gap), inner.origin.y, seg_w, inner.size.h);

        // lit cells are filled solid and empty cells are hollow. a black interior with
        // an accent outline so the cell stays visible without reading as charged
        graphics_context_set_fill_color(ctx, i < lit ? fill_color : GColorBlack);
        graphics_fill_rect(ctx, cell, 1, GCornersAll);

        if (i >= lit)
        {
            graphics_context_set_stroke_color(ctx, accent_color);
            graphics_draw_round_rect(ctx, cell, 1);
        }
    }
}

/**
 * @brief Label definition mapping an area to its string text.
 */
typedef struct
{
    GRect area;
    const char *text;
} LcarsLabel;

/**
 * @brief Blit a transparent icon bitmap, skipping nulls.
 *
 * @param ctx The graphics context.
 * @param bmp The bitmap to draw.
 * @param r The destination rectangle.
 */
static void draw_icon(GContext *ctx, GBitmap *bmp, GRect r)
{
    if (!bmp)
    {
        return;
    }

    graphics_context_set_compositing_mode(ctx, GCompOpSet);  // honor PNG transparency
    graphics_draw_bitmap_in_rect(ctx, bmp, r);
}

void widgets_load(void)
{
    s_font_label = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_10));
}

void widgets_unload(void)
{
    fonts_unload_custom_font(s_font_label);
}

void widgets_draw_labels(GContext *ctx)
{
    const LcarsLabel labels[] = {
        {LBL_STARDATE, "STARDATE"},
        {LBL_WEATHER, "SENSORS"},
        {LBL_HR, "VITALS"},
        {LBL_STEPS, "TRAVERSAL"},
    };

    GColor lbl_color = label_color_for_theme(settings_u8(SETTING_THEME));

    for (unsigned i = 0; i < ARRAY_LENGTH(labels); i++)
    {
        lcars_label(ctx, labels[i].area, labels[i].text, s_font_label, lbl_color, 3);
    }
}

void widgets_draw_battery(GContext *ctx, int level)
{
    uint8_t theme = settings_u8(SETTING_THEME);
    GColor accent = panel_accent_for_theme(theme);
    GColor fill = battery_fill_for_theme(theme, level);
    lcars_battery_gauge(ctx, GRect(6, 4, 32, 12), level, accent, fill);
}

void widgets_draw_glyphs(GContext *ctx, const char *condition, bool bt_show, bool bt_connected)
{
    // the cache loads each glyph once and hands back the same picture on later paints
    draw_icon(ctx, icon_get(wx_resource_for(condition)), WX_ICON);
    draw_icon(ctx, icon_get(RESOURCE_ID_ICON_THERMOMETER), THERMO_ICON);
    draw_icon(ctx, icon_get(RESOURCE_ID_ICON_HEART), HR_ICON);
    draw_icon(ctx, icon_get(RESOURCE_ID_ICON_FEET), FEET_ICON);

    if (bt_show)
    {
        GBitmap *bt = icon_get(bt_connected ? RESOURCE_ID_ICON_BLUETOOTH : RESOURCE_ID_ICON_BLUETOOTH_SLASH);
        icon_tint(bt, GColorBlack);  // the master glyph is white so paint it black to match the panel text
        draw_icon(ctx, bt, BT_ICON);
    }
}

/** @} */
