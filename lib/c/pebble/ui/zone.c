/**
 * @file zone.c
 * @brief Layout primitives implementation
 */
#include "ui/zone.h"

TextLayer *zone_make_layer(Layer *parent, const Zone *zone)
{
    TextLayer *layer = text_layer_create(zone->rect);
    text_layer_set_background_color(layer, GColorClear);
    text_layer_set_font(layer, fonts_get(zone->font_id));
    text_layer_set_text_alignment(layer, zone->align);
    text_layer_set_text_color(layer, zone->color);
    layer_add_child(parent, text_layer_get_layer(layer));
    return layer;
}

// rendered single-line width of text in font (height is irrelevant for the fit test)
static int text_width(const char *text, GFont font, GTextAlignment align)
{
    GSize sz = graphics_text_layout_get_content_size(text, font, GRect(0, 0, 1000, 100),
        GTextOverflowModeTrailingEllipsis, align);
    return sz.w;
}

void zone_set_text_fit(TextLayer *layer, const Zone *zone, const char *text)
{
    // step down through the font tiers largest first and use the largest one
    // whose text fits. a face omits a tier by leaving its rect zero-sized (C
    // zero-fills structs). tier 0 always exists. if nothing fits the smallest
    // defined tier wins and the text layer trails an ellipsis as a last resort
    const FontId fonts[3] = {zone->font_id, zone->font_id_fallback, zone->font_id_fallback2};
    const GRect rects[3] = {zone->rect, zone->rect_fallback, zone->rect_fallback2};

    int chosen = 0;
    for (int tier = 0; tier < 3; tier++)
    {
        // a zero-sized rect means this tier (and any after it) isn't defined
        if (tier > 0 && (rects[tier].size.w == 0 || rects[tier].size.h == 0))
        {
            break;
        }

        chosen = tier;

        // 2px safety margin against the tier's own width
        if (text_width(text, fonts_get(fonts[tier]), zone->align) <= rects[tier].size.w - 2)
        {
            break;
        }
    }

    text_layer_set_font(layer, fonts_get(fonts[chosen]));
    layer_set_frame(text_layer_get_layer(layer), rects[chosen]);
    text_layer_set_text(layer, text);
}
