/**
 * @file stock_wire.h
 * @brief The watchlist as the phone packs it, and the reader that unpacks it.
 *
 * The phone sends the watchlist as one run of bytes with the quotes laid end to end, each carrying
 * its own symbol length. Nothing about it is fixed width, so the reader walks it and checks every
 * step against the length it was handed. The bytes come off a phone, so none of it is trusted.
 *
 * The packer on the other side is lib/ts/pkjs/wire.ts, and the two have to agree exactly.
 *
 * @ingroup lib_core
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

/**
 * @addtogroup lib_core
 * @{
 */

/// How many tickers the watchlist can hold. Matches STOCK_MAX_SLOTS on the phone
#define STOCK_MAX_SLOTS 4

/// Room for a ticker (up to about 6) or a short status word ("INVALID KEY" is 11) plus the NUL
#define STOCK_SYMBOL_LEN 12

/**
 * @brief One quote slot. When ok is false the symbol field carries a short status word
 * ("RATE LIMIT" and friends) instead of a ticker so the watch has something to show.
 */
typedef struct
{
    char symbol[STOCK_SYMBOL_LEN]; ///< Ticker, or status text when ok is false, or empty when there is none
    int  price_cents;              ///< Last price times 100
    int  change_pct;               ///< Percent change times 100, can be negative
    bool ok;                       ///< True for a live reading, false for a status message
} StockSlot;

/** @brief The watchlist strip. count is 0 until a reading lands. */
typedef struct
{
    uint8_t   count;                 ///< How many slots are filled (0 means none yet)
    StockSlot slot[STOCK_MAX_SLOTS];
} StockStrip;

/**
 * @brief Unpacks a watchlist off the wire.
 *
 * Layout is [count] then per slot [ok][price int32 LE cents][pct int16 LE hundredths][symLen]
 * [sym bytes]. A count past what the strip holds is pinned to it rather than walking off the end.
 *
 * Nothing is written to @p out until the whole run reads clean, so a message that stops halfway
 * leaves the caller's last good watchlist alone rather than half replacing it.
 *
 * @param buf The raw wire bytes.
 * @param len How many bytes there are.
 * @param out Receives the watchlist. Untouched unless this returns true.
 * @return Whether the run read clean.
 */
bool stock_wire_decode(const uint8_t *buf, uint16_t len, StockStrip *out);

/** @} */
