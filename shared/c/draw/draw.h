// LCARS drawing helpers shared across watchfaces
#pragma once
#include <pebble.h>

TextLayer *draw_make_layer(Layer *parent, GRect frame, GFont font, GTextAlignment align);
void draw_string_to_upper(char *s);
void draw_lcars_label(GContext *ctx, GRect area, const char *text, GFont font, GColor text_color, int pad);
void draw_lcars_battery_gauge(GContext *ctx, GRect area, int level, GColor accent_color);
