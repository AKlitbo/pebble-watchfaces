/**
 * @file layout_switch.h
 * @brief Keeps the layout on screen in step with whatever should be bringing it in.
 *
 * A mosaic face holds three layouts and this decides which one is showing. Two things can ask for
 * one: the night schedule, and Quiet Time. The picking itself is plain logic and lives in lib
 * (layout/layout_role.h and clock/nightsched.h); what is here is the wiring: reading the clock,
 * the sun and the watch's own Quiet Time setting, and telling the engine to rebuild on the few
 * minutes a day the answer actually changes.
 *
 * Every reading comes from the face through the names in its settings_schema.h, so the two faces
 * share this whole file and differ only in what those hand back.
 *
 * @ingroup mosaic
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup mosaic
 * @{
 */

/**
 * @brief Settle on a layout before the first paint.
 *
 * Call after settings_init (and after the dev overrides) but before engine_init, so a watchface
 * launched at midnight comes up on the night layout rather than flashing the day one first.
 */
void layout_switch_init(void);

/**
 * @brief Re-check after the config page has been through.
 *
 * Call as the first thing in the settings-changed handler: it only moves the role, and the rebuild
 * that handler already does then builds the right grid.
 */
void layout_switch_settings_changed(void);

/**
 * @brief Re-check on the minute tick, and on a fresh weather reading.
 *
 * Rebuilds the engine itself, but only on the few minutes a day the answer actually changes.
 * Weather matters here as well as the clock: a sunset that arrives mid-evening can mean the layout
 * should already have changed, and waiting up to a minute for the next tick would show.
 *
 * Quiet Time is read here too. The SDK offers no way to be told when it flips, so the minute tick
 * is the only place to notice, which also caps how long a panel showing it can read the old value.
 */
void layout_switch_tick(void);

/** @} */
