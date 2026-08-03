/**
 * @file calendar_store.c
 * @brief The active calendar store: holds the agenda, owns the appmessage calendar channel, and
 * polls the phone on its own timer.
 */
#include "io/stores/calendar_store.h"

#include <time.h>

#include "io/appmessage/appmessage.h"
#include "io/stores/store_cadence.h"
#include "io/stores/store_persist.h"
#include "io/stores/store_poll.h"

// a short first fetch after launch (fires from the event loop so appmessage is open by then),
// staggered a touch past weather/stock so the outbox is not contended at boot. then the
// recurring poll runs at the configured interval
#define CALENDAR_FIRST_POLL_MS 1100

static struct
{
    CalendarStrip strip;
    time_t        last_sync;
} s_state;

// the full strip (6 fat events) is too big to persist: it blows past the 256 byte persist key
// ceiling so the save just fails. a relaunch instead saves a trimmed snapshot of the first few
// events plus the sync time, enough to paint the agenda right away before the first poll after
// launch refreshes the full strip
#define CALENDAR_PERSIST_SLOTS 4
typedef struct
{
    uint8_t       tag; // STORE_TAG_CALENDAR, so a restore can tell this blob from another shape
    uint8_t       count;
    CalendarEvent event[CALENDAR_PERSIST_SLOTS];
    time_t        last_sync;
} CalendarPersist;
_Static_assert(sizeof(CalendarPersist) <= PERSIST_DATA_MAX_LENGTH, "calendar snapshot must fit one persist key");

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

// stash a trimmed snapshot so a relaunch can restore it. only a live face writes
static void persist_save(void)
{
    if (!s_live)
    {
        return;
    }

    // keep only the first few events. the rest stay zeroed so the saved bytes are deterministic
    CalendarPersist snap = {0};
    snap.count = s_state.strip.count < CALENDAR_PERSIST_SLOTS ? s_state.strip.count : CALENDAR_PERSIST_SLOTS;
    for (uint8_t i = 0; i < snap.count; i++)
    {
        snap.event[i] = s_state.strip.event[i];
    }
    snap.last_sync = s_state.last_sync;
    store_save(s_persist_key, &snap, sizeof(snap), STORE_TAG_CALENDAR);
}

// prefill the store from a seed (dev/screenshots). s_cb is NULL at init so no redraw here
static void apply_seed(const CalendarSeed *seed)
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
 * @brief Calendar channel. Takes the agenda the reader unpacked and puts it away.
 *
 * A message that does not read clean leaves the last good agenda where it is, so a bad one can
 * never blank the panel.
 *
 * @param buf The raw wire bytes.
 * @param len How many bytes there are.
 */
static void on_calendar_strip(const uint8_t *buf, uint16_t len)
{
    CalendarStrip built;
    if (!calendar_wire_decode(buf, len, &built))
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
 * @brief The one-shot catch-up fetch, used at launch and after a settings save.
 *
 * @param data The timer context (unused).
 */
static void catch_up_fire(void *data)
{
    s_timer = NULL;
    appmessage_request_calendar();
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
        appmessage_request_calendar();
    }
}

// --- public API ---

void calendar_store_subscribe(void (*cb)(void))
{
    s_cb = cb;
}

void calendar_store_init(CalendarConfig cfg, const CalendarSeed *seed)
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
        // restore the snapshot so a relaunch shows the last agenda right away. the first poll then
        // refreshes the full strip in the background
        CalendarPersist snap;
        if (store_restore(s_persist_key, &snap, sizeof(snap), STORE_TAG_CALENDAR))
        {
            uint8_t n = snap.count < CALENDAR_PERSIST_SLOTS ? snap.count : CALENDAR_PERSIST_SLOTS;
            s_state.strip.count = n;
            for (uint8_t i = 0; i < n; i++)
            {
                s_state.strip.event[i] = snap.event[i];
            }
            s_state.last_sync = snap.last_sync;
        }
    }

    if (!cfg.enabled)
    {
        return;
    }

    if (cfg.live)
    {
        // the store owns the calendar channel. faces that don't declare the key never see it fire
        appmessage_on_calendar_strip(on_calendar_strip);

        // one fetch shortly after launch so the agenda is not blank while the first deadline is
        // still coming. poll_min 0 disables polling, matching reconfigure
        stop_polling();
        if (s_poll_min > 0)
        {
            s_timer = app_timer_register(CALENDAR_FIRST_POLL_MS, catch_up_fire, NULL);
        }
    }
}

void calendar_store_reconfigure(CalendarConfig cfg)
{
    s_poll_min = cfg.poll_min;

    stop_polling();
    if (cfg.enabled && cfg.live && s_poll_min > 0)
    {
        s_next_poll = store_poll_next(s_poll_min, time(NULL));

        // pull once shortly after the change, same as init, so a new interval (or any settings
        // save) refreshes the agenda right away instead of waiting for the deadline. an agenda
        // goes stale on the phone's say-so rather than the watch's, so it is worth the fetch
        s_timer = app_timer_register(CALENDAR_FIRST_POLL_MS, catch_up_fire, NULL);
    }
}

const CalendarStrip *calendar_store_strip(void)
{
    return &s_state.strip;
}

const CalendarEvent *calendar_store_event(uint8_t index)
{
    if (index >= s_state.strip.count)
    {
        return NULL;
    }
    return &s_state.strip.event[index];
}

int calendar_store_age_s(void)
{
    return s_state.last_sync ? (int)(time(NULL) - s_state.last_sync) : -1;
}
