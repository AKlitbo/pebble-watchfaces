/**
 * @file stock_wire.c
 * @brief The watchlist as the phone packs it, and the reader that unpacks it.
 */
#include "wire/stock_wire.h"

#include "io/bytes_le.h"
#include "text/number_format.h"

bool stock_wire_decode(const uint8_t *buf, uint16_t len, StockStrip *out)
{
    if (!buf || !out || len < 1)
    {
        return false;
    }

    uint8_t count = buf[0];
    if (count > STOCK_MAX_SLOTS)
    {
        // the phone's word for how many, pinned to how many there is room for. without this the
        // fill below walks straight off the end of the slot array
        count = STOCK_MAX_SLOTS;
    }

    // zero-init so the unused slot[count..MAX-1] bytes are deterministic, not stale
    // stack, once the whole strip gets persisted to flash
    StockStrip built = {0};

    uint16_t offset = 1;
    for (uint8_t i = 0; i < count; i++)
    {
        // the fixed part of a slot is ok(1) + price(4) + pct(2) + symLen(1) = 8 bytes
        if (offset + 8 > len)
        {
            return false; // malformed, the caller keeps the old strip
        }

        StockSlot *slot = &built.slot[i];
        slot->ok = buf[offset] != 0;
        slot->price_cents = read_i32_le(buf + offset + 1);
        slot->change_pct = read_i16_le(buf + offset + 5);
        uint8_t sym_len = buf[offset + 7];
        offset += 8;

        if (offset + sym_len > len)
        {
            return false; // symbol runs past the buffer
        }

        // take what fits, then step past the whole wire symbol however long it was, or a long one
        // would leave the walk pointing into the middle of itself and the next slot reads as noise
        copy_bounded(slot->symbol, STOCK_SYMBOL_LEN, buf + offset, sym_len);
        offset += sym_len;

        built.count++;
    }

    // only now, once the whole run has read clean
    *out = built;
    return true;
}
