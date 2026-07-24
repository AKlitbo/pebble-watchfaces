/**
 * @file zone.h
 * @brief Layout primitives: defines paintable areas and text alignments
 *
 * @ingroup lib_ui
 */
// a Zone is everything needed to present one text slot: where it sits and which
// registered font + alignment + colour to use plus up to two smaller font/rect
// tiers to step down to when the text would overflow. a face declares a static
// const table of these and the engine loops it to build the text layers
#pragma once
#include <pebble.h>

#include "ui/fonts.h"

/**
 * @addtogroup lib_ui
 * @{
 */

/**
 * @brief Defines a paintable area, including geometry and styling.
 */
typedef struct
{
    GRect          rect;
    FontId         font_id;
    GTextAlignment align;
    GColor         color;
    FontId         font_id_fallback;  ///< First smaller font to try when the text overflows the main one
    GRect          rect_fallback;     ///< Area for the first smaller font
    FontId         font_id_fallback2; ///< Second smaller font for the widest strings
    GRect          rect_fallback2;    ///< Area for the second smaller font. Leave it zero-sized to skip this step
} Zone;

/**
 * @brief Create transparent text layer for a zone and attach to parent.
 *
 * @param parent The parent layer.
 * @param zone The zone properties.
 * @return The new text layer, or NULL when the heap had no room for it. A caller holding NULL
 *   has an empty slot, so it must check before passing the layer back in.
 */
TextLayer *zone_make_layer(Layer *parent, const Zone *zone);

/**
 * @brief Set text, shrinking to fallback if overflow.
 *
 * @param layer The text layer.
 * @param zone The zone specifying the fallback font/rect.
 * @param text The text to set.
 */
void zone_set_text_fit(TextLayer *layer, const Zone *zone, const char *text);

/** @} */
