/**
 * @file stock_store.h
 * @brief Active stock store. It owns the appmessage stock channel and its own poll timer,
 * so a face just hands it the rules (enabled / live / poll interval), optionally seeds it,
 * then reads the slots and subscribes for repaints. The phone data flows straight in and
 * nobody wires it.
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

// how many tickers the watchlist can hold. matches STOCK_MAX_SLOTS on the phone
#define STOCK_MAX_SLOTS 4

// room for a ticker (up to ~6) or a short status word ("INVALID KEY" is 11) plus the NUL
#define STOCK_SYMBOL_LEN 12

/**
 * @brief One quote slot. When ok is false the symbol field carries a short status word
 * ("RATE LIMIT" and friends) instead of a ticker so the watch has something to show.
 */
typedef struct
{
    char symbol[STOCK_SYMBOL_LEN]; // ticker, or status text when ok is false, "" when empty
    int  price_cents;              // last price times 100
    int  change_pct;               // percent change times 100, signed
    bool ok;                       // true = a live reading, false = a status message
} StockSlot;

/** @brief The watchlist strip. count is 0 until a reading lands. */
typedef struct
{
    uint8_t   count;                 // how many slots are filled (0 = none yet)
    StockSlot slot[STOCK_MAX_SLOTS];
} StockStrip;

/**
 * @brief The rules a face hands the store, built from its own settings so the store names no
 * SETTING_*.
 */
typedef struct
{
    bool enabled;   // false = inert (no channel no timer holds nothing)
    bool live;      // true = subscribe the channel + poll. false = seed-only (dev/screenshots)
    int  poll_min;  // minutes between stock requests
} StockConfig;

/**
 * @brief Optional prefill: shown until the first real reading lands, or pinned when
 * live = false.
 */
typedef struct
{
    const StockStrip *strip; // the slots to show, or NULL for none
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
