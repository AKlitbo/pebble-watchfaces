#pragma once
#include "mosaic/engine/catalog.h"

/**
 * @file time_analog.h
 * @brief A 2x2 analog clock panel: a circle, twelve ticks, and hour plus minute hands.
 * The header can be turned off per the module appearance editor, which gives the bigger
 * full-tile dial.
 * @ingroup gridlock_mod_time
 */

/** @brief The analog clock panel hooked into the grid so the watchface can show it. */
extern const ModuleDef mod_time_analog_def;
