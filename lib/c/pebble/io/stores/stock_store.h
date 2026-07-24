/**
 * @file stock_store.h
 * @brief Active stock store. It owns the appmessage stock channel and its own poll timer,
 * so a face just hands it the rules (enabled / live / poll interval), optionally seeds it,
 * then reads the slots and subscribes for repaints. The phone data flows straight in and
 * nobody wires it.
 *
 * @ingroup lib_stores
 */
#pragma once
#include <pebble.h>

#include "wire/stock_wire.h"

/**
 * @addtogroup lib_stores
 * @{
 */

/**
 * @brief The rules a face hands the store, built from its own settings so the store names no
 * SETTING_*.
 */
typedef struct
{
    bool     enabled;     ///< False makes the store do nothing (no channel and no timer)
    bool     live;        ///< True subscribes the channel and polls. False just keeps the fake data for screenshots
    int      poll_min;    ///< Minutes between stock requests
    uint32_t persist_key; ///< Slot for the last good strip so the face owns the key instead of the store
} StockConfig;

/**
 * @brief Optional prefill: shown until the first real reading lands, or pinned when
 * live = false.
 */
typedef struct
{
    const StockStrip *strip; ///< The slots to show, or NULL for none
} StockSeed;

/**
 * @brief Start the store with its rules. Pass seed = NULL for normal use.
 *
 * @param cfg The rules (enabled / live / poll interval).
 * @param seed Optional prefill, or NULL.
 */
void stock_store_init(StockConfig cfg, const StockSeed *seed);

/**
 * @brief Re-apply the rules (e.g. the poll interval changed). Re-arms the timer.
 *
 * @param cfg The new rules.
 */
void stock_store_reconfigure(StockConfig cfg);

/** @brief Hand it the function to call when a quote changes (the screen redraw). */
void stock_store_subscribe(void (*cb)(void));

/** @brief The whole watchlist strip. count is 0 until a reading lands. */
const StockStrip *stock_store_strip(void);

/**
 * @brief One slot by index, or NULL when the index is past what is filled.
 *
 * @param index Which slot to read (0 based).
 * @return The slot, or NULL.
 */
const StockSlot *stock_store_slot(uint8_t index);

/** @brief How many seconds since the last reading turned up, or -1 if we have none. */
int stock_store_age_s(void);

/** @} */
