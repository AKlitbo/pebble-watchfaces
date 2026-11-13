/**
 * @file system_battery.h
 * @brief The battery panel, plus the drawing it shares with the other battery panels.
 * @ingroup mosaic_mod_system
 */
#pragma once
#include "mosaic/engine/catalog.h"
#include "ui/fonts.h"

/**
 * @addtogroup mosaic_mod_system
 * @{
 */

/**
 * @brief Draws the battery panel. The charge percent sits next to a little gauge
 * made of segments.
 */
extern const ModuleDef mod_system_battery_def;

/**
 * @brief Draws a battery panel's body for any battery. At 1x2 the level sits on the left with a
 * five segment gauge beside it, and at 2x2 it sits over an OF 100% LEFT line and a ten segment gauge.
 *
 * With no reading it shows -- over an empty gauge, and the 2x2 line reads NO READING.
 *
 * @param gctx The grid context.
 * @param level The level from 0 to 100, or -1 for no reading.
 */
void battery_body(GridCtx *gctx, int level);

/**
 * @brief Draws a battery panel's body for a phone whose plugins are off, so it has no level to show.
 * At 1x2 it reads PLUGINS OFF across the row, and at 2x2 it keeps the dashes and the empty gauge with
 * PLUGINS OFF under the dashes.
 *
 * @param gctx The grid context.
 */
void battery_body_plugins_off(GridCtx *gctx);

#define BATTERY_VALUE_H 30 ///< the height of the box a 1x2 battery number draws in, as the other 1x2 values use

/**
 * @brief How wide a battery number's ink draws in a font, for a panel that picks its own size.
 *
 * @param text The number as the panel writes it.
 * @param font The font it will draw in.
 * @return The width in pixels.
 */
int battery_text_w(const char *text, FontId font);

/** @} */
