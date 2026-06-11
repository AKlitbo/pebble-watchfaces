// overlay widgets API: layers the shell mounts over the baked LCARS frame
#pragma once
#include <pebble.h>

// painted overlays drawn on top of the baked LCARS frame: the battery gauge,
// the amber bar labels, and the weather / heart / feet glyphs. This module owns
// the three overlay layers plus their label font and icon bitmaps - create it
// after the background layer and before the text layers to preserve z-order
void widgets_create(Layer *parent);
void widgets_destroy(void);

void widgets_set_battery(int level);                   // 0..100, redraws the gauge
void widgets_set_weather_icon(const char *condition);  // swaps the wx glyph
void widgets_mark_labels_dirty(void);                  // force a label repaint
