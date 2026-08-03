/**
 * @file stock_store.c
 * @brief The active stock store: holds the quotes, owns the appmessage stock channel, and
 * polls the phone on its own timer.
 */
#include "io/stores/stock_store.h"

#include <time.h>

#include "io/appmessage/appmessage.h"
#include "io/stores/store_cadence.h"
#include "io/stores/store_persist.h"
#include "io/stores/store_poll.h"

// a short first fetch after launch (fires from the event loop so appmessage is open by then)
// then the recurring poll runs at the configured interval
#define STOCK_FIRST_POLL_MS 700

static struct
{
    uint8_t    tag;  // STORE_TAG_STOCK, so a restore can tell this blob from another shape
    StockStrip strip;
    time_t     last_sync;
} s_state;
_Static_assert(sizeof(s_state) <= PERSIST_DATA_MAX_LENGTH, "stock state must fit one persist key");

static void (*s_cb)(void);
static AppTimer *s_timer;  // the catch-up fetch only. the recurring poll rides the cadence
static int s_poll_min;
static time_t s_next_poll; // wall-clock second the next recurring poll is due
static bool s_live;
static uint32_t s_persist_key; // the slot the face handed us for the saved strip

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
        store_save(s_persist_key, &s_state, sizeof(s_state), STORE_TAG_STOCK);
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
 * @brief Stock channel. Takes the watchlist the reader unpacked and puts it away.
 *
 * A message that does not read clean leaves the last good quotes where they are, so a bad one can
 * never blank the watchlist.
 *
 * @param buf The raw wire bytes.
 * @param len How many bytes there are.
 */
static void on_stock_strip(const uint8_t *buf, uint16_t len)
{
    StockStrip built;
    if (!stock_wire_decode(buf, len, &built))
    {
        return;
    }

    s_state.strip = built;
    s_state.last_sync = time(NULL);
    persist_save();
    if (s_cb) s_cb();
}

// --- poll timer ---

/**
 * @brief The one-shot catch-up fetch, used at launch and when the panel turns up empty.
 *
 * @param data The timer context (unused).
 */
static void catch_up_fire(void *data)
{
    s_timer = NULL;
    appmessage_request_stock();
}

static void stop_polling(void)
{
    if (s_timer)
    {
        app_timer_cancel(s_timer);
        s_timer = NULL;
    }
}

/**
 * @brief The store's turn on the face's cadence: poll when the deadline has come round.
 */
static void cadence_poll(void)
{
    if (!s_live)
    {
        return;
    }

    if (store_poll_due(s_poll_min, &s_next_poll, time(NULL)))
    {
        appmessage_request_stock();
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
    s_persist_key = cfg.persist_key;
    reset_state();
    s_poll_min = cfg.poll_min;
    s_next_poll = store_poll_next(s_poll_min > 0 ? s_poll_min : 1, time(NULL));
    store_cadence_register(cadence_poll);

    if (seed)
    {
        apply_seed(seed); // s_cb is NULL until the face subscribes so no redraw yet
    }
    else if (cfg.live)
    {
        // restore the last good strip so a relaunch shows it right away. the first poll
        // then refreshes it in the background
        if (store_restore(s_persist_key, &s_state, sizeof(s_state), STORE_TAG_STOCK))
        {
            // stock_store_slot bounds an index against this count, so pin it to what the slot
            // array actually holds before anything can ask for a slot past the end
            if (s_state.strip.count > STOCK_MAX_SLOTS)
            {
                s_state.strip.count = 0;
            }
        }
    }

    if (!cfg.enabled)
    {
        return;
    }

    if (cfg.live)
    {
        // the store owns the stock channel. faces that don't declare the key never see it fire
        appmessage_on_stock_strip(on_stock_strip);

        // one fetch shortly after launch so the strip is not left on -- while the first deadline
        // is still coming. poll_min 0 disables polling, matching reconfigure
        stop_polling();
        if (s_poll_min > 0)
        {
            s_timer = app_timer_register(STOCK_FIRST_POLL_MS, catch_up_fire, NULL);
        }
    }
}

void stock_store_reconfigure(StockConfig cfg)
{
    s_poll_min = cfg.poll_min;

    stop_polling();
    if (cfg.enabled && cfg.live && s_poll_min > 0)
    {
        s_next_poll = store_poll_next(s_poll_min, time(NULL));

        // an empty store has nothing but -- to draw, so catch up right away: the panel may have
        // just been added to the layout, or this very save may have cancelled the launch poll.
        // one that already holds quotes waits for the deadline, so a save that only touched
        // colours never pulls a fetch forward and spends a metered provider's quota
        if (s_state.strip.count == 0)
        {
            s_timer = app_timer_register(STOCK_FIRST_POLL_MS, catch_up_fire, NULL);
        }
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
