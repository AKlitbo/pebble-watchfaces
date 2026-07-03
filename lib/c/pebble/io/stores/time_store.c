/**
 * @file time_store.c
 * @brief The active time store: holds the current time and owns the tickers that update it.
 */
#include "io/stores/time_store.h"

#include <time.h>

#include "system/units/units.h"

static struct tm s_tm;
static void (*s_cb)(void);

static AppTimer *s_beats_timer; // the .beats ticker. NULL when it isn't running
static bool s_minute;           // minute tick on
static bool s_beats;            // .beats timer on

static void tick_handler(struct tm *tick_time, TimeUnits units_changed);

/**
 * @brief Save the new time and tell whoever is listening.
 *
 * @param t The new time.
 */
static void set(const struct tm *t)
{
    s_tm = *t;
    if (s_cb) s_cb();
}

/**
 * @brief Minute-tick handler: feed the tick straight in.
 */
static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    set(tick_time);
}

/**
 * @brief .beats ticker: refresh the time and re-arm for the next beat boundary.
 *
 * @param data The timer context (unused).
 */
static void beats_fire(void *data)
{
    time_t now = time(NULL);
    set(localtime(&now));

    s_beats_timer = app_timer_register(units_ms_until_next_beat(), beats_fire, NULL);
}

/**
 * @brief Run the tickers the config asks for. The two cadences are independent, so a face can
 * have the minute tick, the .beats timer, or both (modular shows a clock and .beats together).
 */
static void start_ticker(void)
{
    if (s_minute)
    {
        tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    }
    else
    {
        tick_timer_service_unsubscribe();
    }

    if (s_beats)
    {
        if (!s_beats_timer)
        {
            s_beats_timer = app_timer_register(units_ms_until_next_beat(), beats_fire, NULL);
        }
    }
    else if (s_beats_timer)
    {
        app_timer_cancel(s_beats_timer);
        s_beats_timer = NULL;
    }
}

/**
 * @brief Stop both tickers (seed mode / teardown).
 */
static void stop_ticker(void)
{
    tick_timer_service_unsubscribe();
    if (s_beats_timer)
    {
        app_timer_cancel(s_beats_timer);
        s_beats_timer = NULL;
    }
}

void time_store_subscribe(void (*cb)(void))
{
    s_cb = cb;
}

void time_store_init(TimeConfig cfg, const struct tm *seed)
{
    s_minute = cfg.minute_tick;
    s_beats = cfg.beats;

    // seed the current time (or the pinned one) so the first paint has something to draw.
    // s_cb is NULL until the face subscribes so this doesn't redraw
    if (seed)
    {
        s_tm = *seed;
    }
    else
    {
        time_t now = time(NULL);
        s_tm = *localtime(&now);
    }

    if (!cfg.enabled)
    {
        return;
    }

    if (cfg.live)
    {
        start_ticker();
    }
}

void time_store_reconfigure(TimeConfig cfg)
{
    s_minute = cfg.minute_tick;
    s_beats = cfg.beats;

    if (cfg.enabled && cfg.live)
    {
        start_ticker();  // swaps to the right cadence
    }
    else
    {
        stop_ticker();
    }
}

const struct tm *time_store_tm(void)
{
    return &s_tm;
}
