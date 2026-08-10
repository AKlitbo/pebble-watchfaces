/**
 * @file night.h
 * @brief Keeps the layout on screen in step with the night schedule.
 *
 * Gridlock holds two layouts and this decides which one is showing. The decision itself is plain
 * arithmetic and lives in lib (clock/nightsched.h); what is here is the wiring: reading the clock
 * and the sun off their stores, and telling the engine to rebuild on the couple of minutes a day
 * the answer actually changes.
 *
 * @ingroup gridlock_settings
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup gridlock_settings
 * @{
 */

/**
 * @brief Settle on a layout before the first paint.
 *
 * Call after settings_init (and after the dev overrides) but before engine_init, so a watchface
 * launched at midnight comes up on the night layout rather than flashing the day one first.
 */
void night_layout_init(void);

/**
 * @brief Re-check after the config page has been through.
 *
 * Call as the first thing in the settings-changed handler: it only moves the flag, and the rebuild
 * that handler already does then builds the right grid.
 */
void night_layout_settings_changed(void);

/**
 * @brief Re-check on the minute tick, and on a fresh weather reading.
 *
 * Rebuilds the engine itself, but only on the couple of minutes a day the window actually opens or
 * shuts. Weather matters here as well as the clock: a sunset that arrives mid-evening can mean the
 * layout should already have changed, and waiting up to a minute for the next tick would show.
 */
void night_layout_tick(void);

/** @} */
