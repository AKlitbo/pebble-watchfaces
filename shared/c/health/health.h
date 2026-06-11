// health-service accessors (no-op stubs when PBL_HEALTH is absent)
#pragma once
#include <pebble.h>

void health_init(HealthEventHandler handler);
int health_get_current_hr(void);
int health_get_today_steps(void);
int health_get_today_distance(void);
