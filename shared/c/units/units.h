// unit conversion + distance formatting helpers
#pragma once
#include <pebble.h>

int units_swatch_beats(void);
float units_m_to_miles(int meters);
float units_m_to_km(int meters);
void units_format_distance(char *buffer, size_t size, int meters, bool miles);
