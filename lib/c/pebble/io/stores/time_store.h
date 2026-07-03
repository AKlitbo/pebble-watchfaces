/**
 * @file time_store.h
 * @brief Active time store. It owns the tickers (the minute tick and the swatch .beats
 * AppTimer) and swaps between them for the current cadence, so a face just hands it the
 * rules and reads the time back.
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief The rules a face hands the store, built from its own settings so the store names no
 * SETTING_*.
 *
 * The two cadences are independent, so a face can run either or both: the shell faces set
 * exactly one (their time format is a single exclusive choice), while modular runs the minute
 * tick always plus the beats timer whenever a live .beats module is on screen.
 */
typedef struct
{
    bool enabled;     // false = inert
    bool live;        // true = run the tickers. false = seed-only (a frozen clock for screenshots)
    bool minute_tick; // run the minute tick (clock date and most readouts)
    bool beats;       // also run the 86.4s .beats timer (for a live .beats readout)
} TimeConfig;

/**
 * @brief Start the store with its rules. Pass seed = NULL to start from the current time.
 *
 * @param cfg The rules (enabled / live / cadence).
 * @param seed Optional time to pin (dev/screenshots), or NULL for the current time.
 */
void time_store_init(TimeConfig cfg, const struct tm *seed);

/**
 * @brief Re-apply the rules (e.g. the time format changed). Swaps the ticker to match.
 *
 * @param cfg The new rules.
 */
void time_store_reconfigure(TimeConfig cfg);

/** @brief Hand it the function to call when the time changes (the screen redraw). */
void time_store_subscribe(void (*cb)(void));

/** @brief The time we hold right now. */
const struct tm *time_store_tm(void);

/** @} */
