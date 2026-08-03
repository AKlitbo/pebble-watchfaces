/**
 * @file store_cadence.h
 * @brief The shared cadence the stores hang their periodic work on.
 *
 * @ingroup lib_stores
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup lib_stores
 * @{
 */

// room for one entry per store that wants the cadence
#define STORE_CADENCE_MAX 8

/**
 * @brief Ask for @p cb to run each time the face's cadence comes round. A store calls this from
 * its own init, so no face has to wire it up by hand.
 *
 * The cadence is whichever ticker the face runs: the minute tick, or the .beats timer in its
 * place. A face with neither never fires this, so put periodic work here, not work something
 * depends on. Registering the same function twice does nothing.
 *
 * @param cb The work to run. NULL is ignored.
 */
void store_cadence_register(void (*cb)(void));

/**
 * @brief Run everything registered, in the order it was registered. The time store calls this
 * from its ticker, before it tells the face the time moved.
 */
void store_cadence_fire(void);

/** @} */
