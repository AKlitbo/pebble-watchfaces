/**
 * @file dev_ops.h
 * @brief The ops-slot walk: a tap steps both slots through the catalog on a fixed fixture, so
 * every readout can be shot without a phone to pick it.
 *
 * The slot picks are this face's own settings, and the emulator has no phone to send them, so
 * they cannot be walked the way the theme walk walks the theme. This overrides the pick at the
 * point layout reads it instead.
 *
 * Its fixture also fills in the sunrise, wind and forecast readings the shared one deliberately
 * leaves empty, because here a "--" would hide the readout being photographed.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-lcars
 * @{
 */

/**
 * @brief Seed every store with a fixture that gives all of the catalog a real reading.
 *
 * The clock is the walk's own, one time per frame, so this takes no hour and minute the way the
 * shared fixture does.
 */
void dev_ops_seed_stores(void);

/**
 * @brief Paint the first pair and subscribe the tap that advances them.
 *
 * @param apply_theme The face's theme-apply hook.
 */
void dev_ops_init(void (*apply_theme)(void));

/** @brief Drop the tap subscription. */
void dev_ops_deinit(void);

/**
 * @brief What the walk wants a slot to show right now.
 *
 * @param slot 0 for the upper slot, 1 for the lower.
 * @return An OpsId.
 */
uint8_t dev_ops_walk_pick(int slot);

/** @} */
