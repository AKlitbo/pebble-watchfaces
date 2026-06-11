// the shell interface a watchface implements (time, info, weather, coords)
#pragma once
#include <pebble.h>

void shell_init(void);
void shell_deinit(void);

void shell_update_display(void);
void shell_update_time(struct tm *tick_time);
void shell_update_info(int hr, int steps, int distance_m, int battery_level);
void shell_set_weather(const char *temp, const char *condition);
void shell_set_coords(const char *lat, const char *lon);
