/**
 * @file system_phone_battery.h
 * @brief The phone battery panel.
 * @ingroup mosaic_mod_system
 */
#pragma once
#include "mosaic/engine/catalog.h"

/**
 * @brief Draws the phone battery panel. It is the battery panel with the phone's level in it, as
 * the Pebble app's Phone plugin reports it, so the two read as a pair side by side.
 */
extern const ModuleDef mod_system_phone_battery_def;
