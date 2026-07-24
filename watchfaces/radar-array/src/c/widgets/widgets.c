/**
 * @file widgets.c
 * @brief Painted overlays: the static frame labels, the battery gauge, and the bluetooth
 * glyph, drawn over the baked frame by the engine's overlays draw-slot.
 *
 * @ingroup watchface-radar
 */
#include "widgets.h"

#include "layout.h"
#include "ui/fonts.h"
#include "draw/fonts.h"
#include "ui/icon_cache.h"
#include "theme/theme.h"
#include "system/settings/settings.h"

/**
 * @addtogroup watchface-radar
 * @{
 */

/**
 * @brief Label definition mapping an area to its string text.
 */
typedef struct
{
    GRect area;
    const char *text;
} Label;

void widgets_draw_labels(GContext *ctx)
{
    GFont font = fonts_get(FONT_XS);
    GColor accent = panel_accent_for_theme(settings_u8(SETTING_THEME));

    const Label labels[] = {
        {LBL_TEMP,  "TEMP"},
        {LBL_PULSE, "PULSE"},
        {LBL_RANGE, "RANGE"},
    };

    graphics_context_set_text_color(ctx, accent);
    for (unsigned i = 0; i < ARRAY_LENGTH(labels); i++)
    {
        graphics_draw_text(ctx, labels[i].text, font, labels[i].area,
            GTextOverflowModeFill, GTextAlignmentCenter, NULL);
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

    GRect body = BATT_RECT;
    uint8_t theme = settings_u8(SETTING_THEME);
    GColor accent = panel_accent_for_theme(theme);

    int nub_h = body.size.h / 2;
    GRect nub = GRect(body.origin.x + body.size.w, body.origin.y + (body.size.h - nub_h) / 2, 2, nub_h);
    graphics_context_set_fill_color(ctx, accent);
    graphics_fill_rect(ctx, nub, 0, GCornerNone);

    graphics_context_set_stroke_color(ctx, accent);
    graphics_draw_rect(ctx, body);

    // five segments lit from the left by charge level
    const int segments = 5;
    const int gap = 1;
    GRect inner = GRect(body.origin.x + 2, body.origin.y + 1, body.size.w - 4, body.size.h - 2);
    int seg_w = (inner.size.w - (segments - 1) * gap) / segments;
    if (seg_w < 1)
    {
        seg_w = 1;
    }

    int lit = (level * segments + 50) / 100;  // round to nearest segment
    if (lit == 0 && level > 0)
    {
        lit = 1;  // never read empty while there's still charge
    }

    GColor on = battery_fill_for_theme(theme, level);
    graphics_context_set_fill_color(ctx, on);

    for (int i = 0; i < lit; i++)
    {
        // inset 1px top and bottom so a dark margin reads as a line inside the outline
        GRect cell = GRect(inner.origin.x + i * (seg_w + gap), inner.origin.y + 1, seg_w, inner.size.h - 2);
        graphics_fill_rect(ctx, cell, 0, GCornerNone);
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

/** @} */
