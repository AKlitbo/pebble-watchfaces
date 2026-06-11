// low-level LCARS drawing helpers (text layers, labels, battery gauge)
#include "draw.h"

// create a transparent text layer and attach it to parent
TextLayer *draw_make_layer(Layer *parent, GRect frame, GFont font, GTextAlignment align)
{
    TextLayer *layer = text_layer_create(frame);
    text_layer_set_background_color(layer, GColorClear);
    text_layer_set_font(layer, font);
    text_layer_set_text_alignment(layer, align);
    layer_add_child(parent, text_layer_get_layer(layer));
    return layer;
}

// uppercase an ascii string in place
void draw_string_to_upper(char *s)
{
    for (char *p = s; *p; p++)
    {
        if (*p >= 'a' && *p <= 'z')
        {
            *p -= 32;
        }
    }
}

// draw a left-aligned label inside a black LCARS holder box
void draw_lcars_label(GContext *ctx, GRect area, const char *text, GFont font, GColor text_color, int pad)
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

// draw the 5-segment battery gauge, color-coded by level
void draw_lcars_battery_gauge(GContext *ctx, GRect area, int level, GColor accent_color)
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

    const int kSegments = 5, kGap = 2;
    int kSegW = (inner.size.w - (kSegments - 1) * kGap) / kSegments;

    int lit = (level * kSegments + 50) / 100;
    if (lit == 0 && level > 0)
    {
        lit = 1;
    }

    GColor on = (level <= 20) ? GColorRed : (level <= 40) ? GColorChromeYellow : accent_color;
    GColor off = GColorDarkGray;

    for (int i = 0; i < kSegments; i++)
    {
        GRect cell = GRect(inner.origin.x + i * (kSegW + kGap), inner.origin.y, kSegW, inner.size.h);
        graphics_context_set_fill_color(ctx, i < lit ? on : off);
        graphics_fill_rect(ctx, cell, 1, GCornersAll);
    }
}
