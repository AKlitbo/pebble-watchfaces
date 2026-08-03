/**
 * @file health_store.c
 * @brief The active health store: reads the watch's health service and holds the numbers.
 * It owns the service accessors too so it is the single point of truth for health.
 */
#include "io/stores/health_store.h"
#include "io/stores/store_cadence.h"
#include "io/stores/store_persist.h"

#include <string.h>
#include <time.h>

// how many minutes of heart rate the graph shows. nothing to do with an hour having 60 minutes
// even though it lands on the same number
#define HR_HISTORY_MINUTES 60
// HOURS_PER_DAY (24) and MINUTES_PER_HOUR (60) come from the Pebble SDK

static struct
{
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

// the only part worth keeping across a relaunch. every other number is read back off the health
// service before the first paint, but the watch logs heart rate too rarely to rebuild the graph
typedef struct
{
    uint8_t tag; // STORE_TAG_HEALTH, so a restore can tell this blob from another shape
    uint8_t hr_history[HR_HISTORY_MINUTES];
    time_t  hr_last_min;
} HealthSaved;
_Static_assert(sizeof(HealthSaved) <= PERSIST_DATA_MAX_LENGTH, "health blob must fit one persist key");

static void (*s_cb)(void);
static bool s_live; // true once subscribed to the live health service, so the minute poll is a no-op in seed mode
// the two history series cost real work, so only a face that graphs one asks for it. the current
// readings are cheap and every face shows one, so those are always tracked
static bool s_hr_history;
static bool s_step_history;
// these three cost a flash read every time they are asked for, so only a face that shows one pays
static bool s_sleep;
static bool s_active;
static bool s_calories;
// finished hours can never gain another step, so they are read once and kept. these live out here
// because init clears the buckets, and stale beliefs about them would strand the cleared ones
static time_t s_cached_day;
static int s_settled_hours;
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
    time_t now = time(NULL);
    time_t day_start = time_start_of_today();
    struct tm *lt = localtime(&now);
    int cur_hour = lt->tm_hour;

    // a new day, or a clock that jumped backwards, leaves the kept buckets describing hours that
    // are no longer the ones being asked about, so drop the lot and read them again
    if (day_start != s_cached_day || cur_hour < s_settled_hours)
    {
        memset(s_state.step_hourly, 0, sizeof(s_state.step_hourly));
        s_state.step_hours = 0;
        s_cached_day = day_start;
        s_settled_hours = 0;
    }

    if (!metric_available(HealthMetricStepCount, day_start, now))
    {
        return;
    }

    // read the hour in progress, plus any whole hours that went by while nothing was looking. each
    // health_service_get_minute_history is a blocking flash scan, so settled hours stay cached
    for (int h = s_settled_hours; h <= cur_hour && h < HOURS_PER_DAY; h++)
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

    // the hour in progress is never settled, nor is the hour just gone while the clock is on its
    // first minute: the watch writes a minute's record at the top of the minute after it, below
    // this in priority, so on the rollover turn that last record is usually not there yet
    s_settled_hours = (lt->tm_min > 0) ? cur_hour : cur_hour - 1;
    if (s_settled_hours < 0)
    {
        s_settled_hours = 0;
    }

    s_state.step_hours = cur_hour + 1;
#endif
}

// --- state + poller ---

// stash the graph so a relaunch can restore it. only a live face writes, so seed mode never
// touches storage. a burst can land a reading a second and a flash write blocks, so the saves
// are held to one a minute, which is all the graph gains anyway
static void persist_save(void)
{
    static time_t s_saved_min = 0;

    if (!s_live)
    {
        return;
    }

    time_t now_min = time(NULL) / SECONDS_PER_MINUTE;
    if (now_min == s_saved_min)
    {
        return;
    }
    s_saved_min = now_min;

    // zeroed so the padding between fields goes to flash as something settled
    HealthSaved saved;
    memset(&saved, 0, sizeof(saved));
    memcpy(saved.hr_history, s_state.hr_history, sizeof(saved.hr_history));
    saved.hr_last_min = s_state.hr_last_min;

    store_save(s_persist_key, &saved, sizeof(saved), STORE_TAG_HEALTH);
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
 * @brief Refresh the heart rate, and append the live reading to the rolling window when the face
 * asked for one. A heart rate is never really 0, so 0 is kept as "no reading yet".
 *
 * The chart reads from this window rather than the minute history: the watch only logs a
 * per-minute heart_rate_bpm now and then in the background, so the minute history is mostly
 * empty, while peeking the live value on each event fills the window with real readings.
 */
static void refresh_hr(void)
{
    int hr = read_hr();
    s_state.hr = hr > 0 ? hr : -1;

    // a face showing a plain heart rate number has no use for the window behind it
    if (!s_hr_history)
    {
        return;
    }

    hr_history_advance(time(NULL) / SECONDS_PER_MINUTE);
    if (hr > 0)
    {
        s_state.hr_history[HR_HISTORY_MINUTES - 1] = (uint8_t)(hr > 255 ? 255 : hr);
        // save on a real reading only, not every minute slide. the restore re-ages the window
        // so the empty minutes in between do not need writing. a burst can still land a reading
        // a second, so persist_save holds the actual writing down to one a minute
        persist_save();
    }
}

/**
 * @brief Refresh the daily activity scalars (steps, distance, calories, sleep, active).
 *
 * These are whole-day sums that move once a minute at most, but movement events arrive every few
 * seconds while walking, and every metric bar steps costs a blocking flash read. So the reads are
 * held to one round a minute, and the three a face has to ask for are skipped unless it has.
 *
 * @param force Read now regardless of the minute gate, for the seed read and for the significant
 * update that calls every number stale.
 * @return True when the numbers were actually re-read, so the caller knows there is something
 * new to repaint.
 */
static bool refresh_activity(bool force)
{
    static time_t s_read_min = 0;
    time_t now_min = time(NULL) / SECONDS_PER_MINUTE;
    if (!force && now_min == s_read_min)
    {
        return false;
    }
    s_read_min = now_min;

    // sleep and active time come in seconds. keep the -1 that means no data rather than
    // dividing it down to 0
    if (s_sleep)
    {
        int sleep_sec = read_sum_today(HealthMetricSleepSeconds, -1);
        s_state.sleep_min = sleep_sec < 0 ? -1 : sleep_sec / 60;
    }

    if (s_active)
    {
        int active_sec = read_sum_today(HealthMetricActiveSeconds, -1);
        s_state.active_min = active_sec < 0 ? -1 : active_sec / 60;
    }

    if (s_calories)
    {
        s_state.calories = read_sum_today(HealthMetricActiveKCalories, -1);
    }

    // the watch keeps the step count to hand so that one is cheap. the distance is not, but it
    // stays ungated because the steps readout can be switched to show it instead
    s_state.steps = read_sum_today(HealthMetricStepCount, 0);
    s_state.distance_m = read_sum_today(HealthMetricWalkedDistanceMeters, 0);

    // the hourly buckets are the dearest thing here, so only a face that graphs them pays
    if (s_step_history)
    {
        read_step_hourly();
    }

    return true;
}

/**
 * @brief Health event handler: refresh only the metrics the event touched, then notify.
 *
 * A movement event does not change the heart rate history and an hr event does not change the
 * step count, so each reads only its own group. The minute gate inside refresh_activity does the
 * rest, turning most movement events into nothing at all.
 *
 * @param event The health event type.
 * @param context Context (unused).
 */
static void on_health_event(HealthEventType event, void *context)
{
    // a movement event that the minute gate turns away read nothing, so there is no repaint to
    // ask for either. the other events always bring something new
    bool changed = true;

    switch (event)
    {
        case HealthEventHeartRateUpdate:
            refresh_hr();
            break;
        case HealthEventMovementUpdate:
            changed = refresh_activity(false);
            break;
        case HealthEventSignificantUpdate:
            refresh_hr();
            refresh_activity(true);
            break;
        default:
            return;
    }

    if (changed && s_cb) s_cb();
}

/**
 * @brief The store's turn on the face's cadence, registered at init so no face wires it by hand.
 *
 * The events alone are not enough. Heart rate events are sparse, so the graph would be dots not a
 * line, and movement events stop the moment the wearer goes still, stranding the step count. The
 * gates inside the two refreshes make this free on a turn an event already covered.
 */
static void cadence_poll(void)
{
    if (!s_live)
    {
        return;
    }

    refresh_hr();
    refresh_activity(false);

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
    s_hr_history = cfg.hr_history;
    s_step_history = cfg.step_history;
    s_sleep = cfg.sleep;
    s_active = cfg.active;
    s_calories = cfg.calories;
    s_persist_key = cfg.persist_key;

    // registering here rather than leaving it to the face means every face gets it
    store_cadence_register(cadence_poll);

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
    // clearing the buckets without clearing what the reader believes about them would leave every
    // hour before s_settled_hours reading zero for the rest of the day
    s_cached_day = 0;
    s_settled_hours = 0;

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
    else if (cfg.live && s_hr_history)
    {
        // restore the last graph so a relaunch (navigating away and back) shows it right away.
        // the tag store_restore checks tells this blob apart from an older shape, so a struct
        // change drops the old one instead of reading it as the wrong thing
        HealthSaved saved;
        if (store_restore(s_persist_key, &saved, sizeof(saved), STORE_TAG_HEALTH))
        {
            memcpy(s_state.hr_history, saved.hr_history, sizeof(s_state.hr_history));
            s_state.hr_last_min = saved.hr_last_min;
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
        // only a face that graphs the window pays for one. the backfill below is a blocking read
        // of the watch's minute log, so it is worth skipping outright for a face that does not
        if (s_hr_history)
        {
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
        }

        // seed the first reading before any event lands
        refresh_hr();
        refresh_activity(true);
    }
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
