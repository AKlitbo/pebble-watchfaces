/**
 * @file health_store.c
 * @brief The active health store: reads the watch's health service and holds the numbers.
 * It owns the service accessors too so it is the single point of truth for health.
 */
#include "io/stores/health_store.h"
#include "io/stores/store_persist.h"

#include <string.h>
#include <time.h>

// how many minutes of heart rate the graph shows. nothing to do with an hour having 60 minutes
// even though it lands on the same number
#define HR_HISTORY_MINUTES 60
// HOURS_PER_DAY (24) and MINUTES_PER_HOUR (60) come from the Pebble SDK

static struct
{
    uint8_t  tag; // STORE_TAG_HEALTH, so a restore can tell this blob from another shape
    int      hr;
    uint8_t  hr_history[HR_HISTORY_MINUTES];
    time_t   hr_last_min; // wall-clock minute of the last history write, so the window can slide
    int      steps;
    int      calories;
    int      sleep_min;
    int      active_min;
    int      distance_m;
    uint16_t step_hourly[HOURS_PER_DAY]; // steps in each hour of today, midnight first
    int      step_hours;                 // how many of those hours are real (0 to 24)
} s_state;
_Static_assert(sizeof(s_state) <= PERSIST_DATA_MAX_LENGTH, "health state must fit one persist key");

static void (*s_cb)(void);
static bool s_live; // true once subscribed to the live health service, so the minute poll is a no-op in seed mode
static uint32_t s_persist_key; // the slot the face handed us for the saved history

// --- health service reads (no-op stubs without PBL_HEALTH) ---

#if defined(PBL_HEALTH)
static bool metric_available(HealthMetric metric, time_t start, time_t end)
{
    HealthServiceAccessibilityMask access = health_service_metric_accessible(metric, start, end);
    return access & HealthServiceAccessibilityMaskAvailable;
}
#endif

static int read_hr(void)
{
#if defined(PBL_HEALTH)
    time_t now = time(NULL);
    if (metric_available(HealthMetricHeartRateBPM, now, now))
    {
        return (int)health_service_peek_current_value(HealthMetricHeartRateBPM);
    }
#endif
    return 0;
}

// sums a metric over today, or returns the fallback when it is unavailable. steps and distance
// pass 0 (a real zero reads fine), the rest pass -1 so a panel can tell "no data yet" apart
// from a real zero and show a placeholder
static int read_sum_today(HealthMetric metric, int fallback)
{
#if defined(PBL_HEALTH)
    time_t now = time(NULL);
    if (metric_available(metric, time_start_of_today(), now))
    {
        return (int)health_service_sum_today(metric);
    }
#else
    (void)metric;
#endif
    return fallback;
}

#if defined(PBL_HEALTH)
// one shared scratch buffer for both minute-history reads (the HR backfill and the step hourly sum).
// they run sequentially on the app thread and each fully drains it before the next, so they never
// need separate storage. that saves a malloc and free on every event
static HealthMinuteData s_minute_scratch[HR_HISTORY_MINUTES];

// the step sum reads a whole hour in one go so the scratch has to hold a full hour of records
// HR_HISTORY_MINUTES is a separate call about how much heart rate the graph shows so the two
// only line up by luck. this fails the build if that window ever shrinks
_Static_assert(ARRAY_LENGTH(s_minute_scratch) >= MINUTES_PER_HOUR,
               "minute scratch must hold a full hour for the step read");
#endif

static void read_hr_history(uint8_t *history_out, int max_records)
{
#if defined(PBL_HEALTH)
    if (max_records <= 0 || !history_out) return;

    // the store only ever asks for the 60 minute window so cap the query to the scratch buffer
    if (max_records > (int)ARRAY_LENGTH(s_minute_scratch))
    {
        max_records = (int)ARRAY_LENGTH(s_minute_scratch);
    }

    memset(history_out, 0, max_records);
    time_t end = time(NULL);
    time_t start = end - (max_records * SECONDS_PER_MINUTE);

    uint32_t records_read = health_service_get_minute_history(s_minute_scratch, max_records, &start, &end);

    // the service is handed the room it has and documents that it returns that many or fewer, but
    // that is its promise rather than something checked here. more than asked for would put offset
    // below zero and the fill would run backwards out of the front of history_out, so take its word
    // no further than the buffer goes
    if (records_read > (uint32_t)max_records)
    {
        records_read = (uint32_t)max_records;
    }

    // the readings come back oldest first and the graph wants them ending at now, so a short read
    // sits at the back and leaves the front zeroed
    int offset = max_records - (int)records_read;
    for (uint32_t i = 0; i < records_read; i++)
    {
        history_out[offset + i] = s_minute_scratch[i].is_invalid ? 0 : s_minute_scratch[i].heart_rate_bpm;
    }
#endif
}

#if defined(PBL_HEALTH)
// squeezes a health sum into a bucket, keeping it in the 0 to 65535 a uint16 can hold
static uint16_t clamp_u16(int value)
{
    if (value < 0) return 0;
    return (uint16_t)(value > 65535 ? 65535 : value);
}
#endif

// fills the per-hour step buckets from midnight up to the current hour. it reads the real
// minute by minute step counts and adds each minute into its hour. health_service_sum can not
// do this: it only hands back a daily total sliced by how long the window is, so every whole
// hour would come out the same. hours yet to come stay 0. step_hours records how many are real
// so the chart can tell a quiet hour from an hour that has not happened
static void read_step_hourly(void)
{
#if defined(PBL_HEALTH)
    // minute data only changes once a minute and movement events fire far more often than that
    // so skip the rebuild until the clock minute moves on. the buckets stay good until then
    static time_t s_built_min = 0;
    time_t now = time(NULL);
    time_t now_min = now / SECONDS_PER_MINUTE;
    if (now_min == s_built_min)
    {
        return;
    }
    s_built_min = now_min;

    memset(s_state.step_hourly, 0, sizeof(s_state.step_hourly));
    s_state.step_hours = 0;

    time_t day_start = time_start_of_today();
    if (!metric_available(HealthMetricStepCount, day_start, now))
    {
        return;
    }

    struct tm *lt = localtime(&now);
    int cur_hour = lt->tm_hour;

    // read a single hour of per-minute records into the shared scratch buffer
    for (int h = 0; h <= cur_hour && h < HOURS_PER_DAY; h++)
    {
        time_t q_start = day_start + (time_t)h * SECONDS_PER_HOUR;
        time_t q_end = q_start + SECONDS_PER_HOUR;
        if (q_end > now)
        {
            q_end = now;
        }

        // every record the service hands back sits inside this hour, so just add up their steps
        uint32_t got = health_service_get_minute_history(s_minute_scratch, MINUTES_PER_HOUR,
                                                         &q_start, &q_end);
        int sum = 0;
        for (uint32_t i = 0; i < got; i++)
        {
            if (!s_minute_scratch[i].is_invalid)
            {
                sum += s_minute_scratch[i].steps;
            }
        }
        s_state.step_hourly[h] = clamp_u16(sum);
    }

    s_state.step_hours = cur_hour + 1;
#endif
}

// --- state + poller ---

// stash the whole state so a relaunch can restore the graph. only a live face writes, so seed
// mode never touches storage
static void persist_save(void)
{
    if (s_live)
    {
        store_save(s_persist_key, &s_state, sizeof(s_state), STORE_TAG_HEALTH);
    }
}

/**
 * @brief Slide the rolling heart rate window forward to now_min, zeroing any minutes that
 * passed with no reading. The last slot always holds the current minute, so the chart reads
 * oldest on the left and newest on the right.
 *
 * @param now_min The current wall-clock minute (time / SECONDS_PER_MINUTE).
 */
static void hr_history_advance(time_t now_min)
{
    time_t elapsed = now_min - s_state.hr_last_min;
    if (elapsed <= 0)
    {
        // same minute, or the clock jumped back. keep the window as it is
        return;
    }

    if (elapsed >= HR_HISTORY_MINUTES)
    {
        memset(s_state.hr_history, 0, sizeof(s_state.hr_history));
    }
    else
    {
        int keep = HR_HISTORY_MINUTES - (int)elapsed;
        memmove(s_state.hr_history, s_state.hr_history + elapsed, keep);
        memset(s_state.hr_history + keep, 0, (int)elapsed);
    }

    s_state.hr_last_min = now_min;
}

/**
 * @brief Refresh the heart rate and append the live reading to the rolling window. A heart
 * rate is never really 0, so 0 is kept as "no reading yet".
 *
 * The chart reads from this window rather than the minute history: the watch only logs a
 * per-minute heart_rate_bpm now and then in the background, so the minute history is mostly
 * empty, while peeking the live value on each event fills the window with real readings.
 */
static void refresh_hr(void)
{
    int hr = read_hr();
    s_state.hr = hr > 0 ? hr : -1;

    hr_history_advance(time(NULL) / SECONDS_PER_MINUTE);
    if (hr > 0)
    {
        s_state.hr_history[HR_HISTORY_MINUTES - 1] = (uint8_t)(hr > 255 ? 255 : hr);
        // save on a real reading only, not every minute slide. the restore re-ages the window
        // so the empty minutes in between do not need writing, which keeps the flash writes rare
        persist_save();
    }
}

/**
 * @brief Refresh the daily activity scalars (steps, distance, calories, sleep, active).
 */
static void refresh_activity(void)
{
    // sleep and active time come in seconds. keep the -1 that means no data rather than
    // dividing it down to 0
    int sleep_sec = read_sum_today(HealthMetricSleepSeconds, -1);
    s_state.sleep_min = sleep_sec < 0 ? -1 : sleep_sec / 60;

    int active_sec = read_sum_today(HealthMetricActiveSeconds, -1);
    s_state.active_min = active_sec < 0 ? -1 : active_sec / 60;

    s_state.steps = read_sum_today(HealthMetricStepCount, 0);
    s_state.calories = read_sum_today(HealthMetricActiveKCalories, -1);
    s_state.distance_m = read_sum_today(HealthMetricWalkedDistanceMeters, 0);

    read_step_hourly();
}

/**
 * @brief Health event handler: refresh only the metrics the event touched, then notify.
 *
 * A movement event does not change the heart rate history and an hr event does not change
 * the step count, so reading just the affected group keeps these frequent events cheap.
 *
 * @param event The health event type.
 * @param context Context (unused).
 */
static void on_health_event(HealthEventType event, void *context)
{
    switch (event)
    {
        case HealthEventHeartRateUpdate:
            refresh_hr();
            break;
        case HealthEventMovementUpdate:
            refresh_activity();
            break;
        case HealthEventSignificantUpdate:
            refresh_hr();
            refresh_activity();
            break;
        default:
            return;
    }

    if (s_cb) s_cb();
}

// --- public API ---

void health_store_subscribe(void (*cb)(void))
{
    s_cb = cb;
}

void health_store_init(HealthConfig cfg, const HealthSeed *seed)
{
    s_live = false;
    s_persist_key = cfg.persist_key;

    // -1 means no reading yet so a panel shows a placeholder until data turns up
    s_state.hr = -1;
    s_state.steps = -1;
    s_state.calories = -1;
    s_state.sleep_min = -1;
    s_state.active_min = -1;
    s_state.distance_m = 0;
    s_state.hr_last_min = 0; // 0 means nothing restored yet, so the live path knows to backfill
    memset(s_state.hr_history, 0, sizeof(s_state.hr_history));
    s_state.step_hours = 0;
    memset(s_state.step_hourly, 0, sizeof(s_state.step_hourly));

    if (seed)
    {
        s_state.hr = seed->hr > 0 ? seed->hr : -1;
        s_state.steps = seed->steps;
        s_state.calories = seed->calories;
        s_state.sleep_min = seed->sleep_min;
        s_state.active_min = seed->active_min;
        s_state.distance_m = seed->distance_m;
        if (seed->hr_history)
        {
            memcpy(s_state.hr_history, seed->hr_history, sizeof(s_state.hr_history));
        }
        s_state.step_hours = seed->step_hours;
        if (seed->step_hourly)
        {
            memcpy(s_state.step_hourly, seed->step_hourly, sizeof(s_state.step_hourly));
        }
    }
    else if (cfg.live)
    {
        // restore the last graph so a relaunch (navigating away and back) shows it right away.
        // the tag store_restore checks tells this blob apart from an older shape, so a struct
        // change drops the old one instead of reading it as the wrong thing
        if (store_restore(s_persist_key, &s_state, sizeof(s_state), STORE_TAG_HEALTH))
        {
            // step_hours indexes the hourly array, so pin it to a real hour count off flash
            if (s_state.step_hours < 0 || s_state.step_hours > HOURS_PER_DAY)
            {
                s_state.step_hours = 0;
            }
        }
    }

    if (!cfg.enabled)
    {
        return;
    }

    if (cfg.live)
    {
        s_live = true;
#if defined(PBL_HEALTH)
        health_service_events_subscribe(on_health_event, NULL);
#endif
        if (s_state.hr_last_min != 0)
        {
            // restored an earlier graph, so just slide it forward over the time we were away
            // (and it clears itself if that was over an hour)
            hr_history_advance(time(NULL) / SECONDS_PER_MINUTE);
        }
        else
        {
            // first launch with nothing saved: backfill from whatever the watch already logged
            // so the chart is not empty, then let the live readings extend it from here
            s_state.hr_last_min = time(NULL) / SECONDS_PER_MINUTE;
            read_hr_history(s_state.hr_history, HR_HISTORY_MINUTES);
        }

        // seed the first reading before any event lands
        refresh_hr();
        refresh_activity();
    }
}

void health_store_poll_hr(void)
{
    if (!s_live)
    {
        return;
    }

    // sample the live heart rate every minute so the graph draws a continuous line. the watch
    // only raises a heart rate event now and then, which would leave the chart as a few stray
    // dots, but peeking the filtered value each minute fills the window minute by minute
    refresh_hr();
    if (s_cb) s_cb();
}

uint8_t *health_store_hr_history(void) { return s_state.hr_history; }

const uint16_t *health_store_step_hourly(void) { return s_state.step_hourly; }
int health_store_step_hours(void)              { return s_state.step_hours; }

int health_store_hr(void)         { return s_state.hr; }
int health_store_steps(void)      { return s_state.steps; }
int health_store_calories(void)   { return s_state.calories; }
int health_store_sleep_min(void)  { return s_state.sleep_min; }
int health_store_active_min(void) { return s_state.active_min; }
int health_store_distance_m(void) { return s_state.distance_m; }
