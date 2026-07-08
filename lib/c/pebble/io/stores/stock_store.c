/**
 * @file stock_store.c
 * @brief The active stock store: holds the quotes, owns the appmessage stock channel, and
 * polls the phone on its own timer.
 */
#include "io/stores/stock_store.h"

#include <time.h>

#include "io/appmessage/appmessage.h"

// a short first fetch after launch (fires from the event loop so appmessage is open by then)
// then the recurring poll runs at the configured interval
#define STOCK_FIRST_POLL_MS 700

// persist slot for the last good strip so a relaunch shows it straight away instead of
// blanking. sits just below the weather store's 255 and well clear of the settings keys
#define STOCK_STORE_PERSIST_KEY 254

static struct
{
    StockStrip strip;
    time_t     last_sync;
} s_state;

static void (*s_cb)(void);
static AppTimer *s_timer;
static int s_poll_ms;
static bool s_live;

// reads a little-endian signed 16-bit value out of a byte pair
static int16_t read_i16(const uint8_t *p)
{
    return (int16_t)(p[0] | (p[1] << 8));
}

// reads a little-endian signed 32-bit value out of four bytes
static int32_t read_i32(const uint8_t *p)
{
    return (int32_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

// --- state writers (internal: only the channel handler + the seed touch these) ---

static void reset_state(void)
{
    s_state.strip.count = 0;
    s_state.last_sync = 0;
}

// stash the whole state so a relaunch can restore it. only a live face writes
static void persist_save(void)
{
    if (s_live)
    {
        persist_write_data(STOCK_STORE_PERSIST_KEY, &s_state, sizeof(s_state));
    }
}

// prefill the store from a seed (dev/screenshots). s_cb is NULL at init so no redraw here
static void apply_seed(const StockSeed *seed)
{
    if (seed->strip)
    {
        s_state.strip = *seed->strip;
    }
    s_state.last_sync = time(NULL);
    if (s_cb) s_cb();
}

// --- appmessage channel handler (the store owns its own wiring) ---

/**
 * @brief Stock channel. Unpacks the wire bytes into the strip.
 *
 * Wire layout is [count] then per slot [ok][price int32 LE cents][pct int16 LE
 * hundredths][symLen][sym bytes]. Slots are variable length so we walk the buffer and bounds
 * check each step. A short or malformed buffer is dropped so a bad message can never leave a
 * half-filled strip. The strip is built in a local first and only committed once it parses
 * clean so a mid-parse bail keeps the last good reading.
 *
 * @param buf The raw wire bytes.
 * @param len How many bytes there are.
 */
static void on_stock_strip(const uint8_t *buf, uint16_t len)
{
    if (!buf || len < 1)
    {
        return;
    }

    uint8_t count = buf[0];
    if (count > STOCK_MAX_SLOTS)
    {
        count = STOCK_MAX_SLOTS;
    }

    StockStrip built;
    built.count = 0;

    uint16_t offset = 1;
    for (uint8_t i = 0; i < count; i++)
    {
        // the fixed part of a slot is ok(1) + price(4) + pct(2) + symLen(1) = 8 bytes
        if (offset + 8 > len)
        {
            return; // malformed, keep the old strip
        }

        StockSlot *slot = &built.slot[i];
        slot->ok = buf[offset] != 0;
        slot->price_cents = read_i32(buf + offset + 1);
        slot->change_pct = read_i16(buf + offset + 5);
        uint8_t sym_len = buf[offset + 7];
        offset += 8;

        if (offset + sym_len > len)
        {
            return; // symbol runs past the buffer
        }

        // copy what fits in the slot buffer but always step past the whole wire symbol
        uint8_t copy = sym_len;
        if (copy > STOCK_SYMBOL_LEN - 1)
        {
            copy = STOCK_SYMBOL_LEN - 1;
        }
        for (uint8_t c = 0; c < copy; c++)
        {
            slot->symbol[c] = (char)buf[offset + c];
        }
        slot->symbol[copy] = '\0';
        offset += sym_len;

        built.count++;
    }

    s_state.strip = built;
    s_state.last_sync = time(NULL);
    persist_save();
    if (s_cb) s_cb();
}

// --- poll timer ---

/**
 * @brief Ask the phone for fresh quotes, then re-arm at the configured interval.
 */
static void poll_fire(void *data)
{
    appmessage_request_stock();
    s_timer = app_timer_register(s_poll_ms, poll_fire, NULL);
}

static void stop_polling(void)
{
    if (s_timer)
    {
        app_timer_cancel(s_timer);
        s_timer = NULL;
    }
}

// --- public API ---

void stock_store_subscribe(void (*cb)(void))
{
    s_cb = cb;
}

void stock_store_init(StockConfig cfg, const StockSeed *seed)
{
    s_live = cfg.live; // set before any persist_save so it knows whether to write
    reset_state();
    s_poll_ms = cfg.poll_min * 60 * 1000;

    if (seed)
    {
        apply_seed(seed); // s_cb is NULL until the face subscribes so no redraw yet
    }
    else if (cfg.live
             && persist_exists(STOCK_STORE_PERSIST_KEY)
             && persist_get_size(STOCK_STORE_PERSIST_KEY) == (int)sizeof(s_state))
    {
        // restore the last good strip so a relaunch shows it right away. the first poll
        // then refreshes it in the background
        persist_read_data(STOCK_STORE_PERSIST_KEY, &s_state, sizeof(s_state));
    }

    if (!cfg.enabled)
    {
        return;
    }

    if (cfg.live)
    {
        // the store owns the stock channel. faces that don't declare the key never see it fire
        appmessage_on_stock_strip(on_stock_strip);

        // first fetch shortly after launch then every poll interval
        stop_polling();
        s_timer = app_timer_register(STOCK_FIRST_POLL_MS, poll_fire, NULL);
    }
}

void stock_store_reconfigure(StockConfig cfg)
{
    s_poll_ms = cfg.poll_min * 60 * 1000;

    stop_polling();
    if (cfg.enabled && cfg.live && s_poll_ms > 0)
    {
        s_timer = app_timer_register(s_poll_ms, poll_fire, NULL);
    }
}

const StockStrip *stock_store_strip(void)
{
    return &s_state.strip;
}

const StockSlot *stock_store_slot(uint8_t index)
{
    if (index >= s_state.strip.count)
    {
        return NULL;
    }
    return &s_state.strip.slot[index];
}

int stock_store_age_s(void)
{
    return s_state.last_sync ? (int)(time(NULL) - s_state.last_sync) : -1;
}
