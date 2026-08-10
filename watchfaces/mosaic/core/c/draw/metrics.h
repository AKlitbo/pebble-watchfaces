/**
 * @file metrics.h
 * @brief The known, steady size and offset facts for the fonts THIS face loads. Think
 * of it as a little book of known offsets so a font's invisible padding is something we
 * look up in one place.
 *
 * The numbers belong to this face only. They describe the real fonts main.c hooks up
 * to each FontId (Teko at a few sizes and Share Tech Mono for the small labels). A
 * different face that hooks different fonts to the same ids keeps its own table. That
 * is why this lives in the face and not in the shared lib.
 *
 * A custom font has some see-through padding above its cap line, so text drawn into a
 * box sits a few pixels lower than the box would suggest. top_pad is that padding. To
 * make the caps line up with the top of a box, draw the text box shifted up by
 * top_pad. The Teko fonts have about 3px and the Share Tech Mono fonts have none.
 *
 * @ingroup mosaic_draw
 */
#pragma once
#include <pebble.h>
#include "ui/fonts.h"

/**
 * @addtogroup mosaic_draw
 * @{
 */

/**
 * @brief The steady pixel facts for one font.
 *
 * top_pad: the see-through rows above the cap line. this is the fix we apply.
 * cap_h:   the measured cap height. rough until we check it in the emulator.
 * line_h:  the rough line height which is the font's point size.
 */
typedef struct
{
    int8_t  top_pad;
    uint8_t cap_h;
    uint8_t line_h;
} FontMetric;

/**
 * @brief The facts record for a font id.
 *
 * @param id The font id.
 * @return The facts for it, or an all-zero record if the id is not known.
 */
const FontMetric *metric_for(FontId id);

/**
 * @brief Takes a text box you have already placed and shifts it up by the font's top
 * padding.
 *
 * Use it after grid_anchor or any other placement so the letters you can see land
 * where the box says they should. This is the one place that holds the "-3" Teko nudge.
 *
 * @param id The font the text will be drawn in.
 * @param anchored The text box as placed.
 * @return The box shifted up by the font's top_pad.
 */
GRect metric_baseline(FontId id, GRect anchored);

/** @} */
