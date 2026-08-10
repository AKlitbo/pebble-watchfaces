/**
 * @file reel.h
 * @brief The column of minutes and the scroll that carries a new one into the centre.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sidereel
 * @{
 */

/**
 * @brief Point the reel at a minute.
 *
 * A step of exactly one minute scrolls when `animate` is set. Anything else is a clock jump
 * (a timezone change, a manual set, the first paint) and snaps instead, or the reel would spin
 * through forty rows to get there.
 *
 * @param minute The minute to centre, 0 to 59.
 * @param animate Whether a one-minute step is allowed to scroll.
 */
void reel_set_minute(int minute, bool animate);

/**
 * @brief Settle any scroll in flight.
 *
 * Must run before engine_rebuild and in teardown: the scroll timer marks the reel's layer dirty,
 * and that layer is about to go away.
 */
void reel_cancel(void);

/**
 * @brief Paint the reel rows. Matches the engine's draw-slot signature.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the reel panel).
 * @param data Unused.
 */
void reel_draw(GContext *ctx, GRect bounds, const void *data);

/** @} */
